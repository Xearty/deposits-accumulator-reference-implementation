// clang-format off
#include <iostream>
#include <assert.h>
#include <bitset>

#include "types.h"

const u64 CURRENT_EPOCH = 100;
const u64 FAR_AWAY_EPOCH = -1;

Node create_leaf_node(Pubkey pubkey, u64 deposit_index, u64 balance, bool signature_is_valid, ValidatorEpochData epoch_data) {
    u64 non_activated_validators_count = CURRENT_EPOCH < epoch_data.activation_epoch;
    u64 active_validators_count = CURRENT_EPOCH >= epoch_data.activation_epoch && CURRENT_EPOCH < epoch_data.exit_epoch;
    u64 exited_validators_count = CURRENT_EPOCH >= epoch_data.exit_epoch;

    Bitset<3> validator_status_bits;
    validator_status_bits.set(NON_ACTIVATED_VALIDATORS_COUNT_BIT, non_activated_validators_count);
    validator_status_bits.set(ACTIVE_VALIDATORS_COUNT_BIT, active_validators_count);
    validator_status_bits.set(EXITED_VALIDATORS_COUNT_BIT, exited_validators_count);

    BoundsData bounds_data {
        ValidatorData {
            pubkey,
            balance,
            validator_status_bits,
        },
        deposit_index,
        signature_is_valid,
    };

    return Node {
        .leftmost = bounds_data,
        .rightmost = bounds_data,
        .accumulated_balance = signature_is_valid ? balance : 0,
        .validator_stats = {
            .non_activated_validators_count = non_activated_validators_count && signature_is_valid,
            .active_validators_count = active_validators_count && signature_is_valid,
            .exited_validators_count = exited_validators_count && signature_is_valid,
        }
    };
}

bool has_same_pubkey_and_is_counted(Pubkey pubkey, const BoundsData& data) {
    return pubkey == data.validator.pubkey && data.counted;
}

bool pubkeys_are_same_and_are_counted(const BoundsData& first, const BoundsData& second) {
    bool pubkeys_are_same = first.validator.pubkey == second.validator.pubkey;
    bool both_are_counted = first.counted && second.counted;
    return pubkeys_are_same && both_are_counted;
}

void inherit_bounds_data_from_children(Node& node, const Node& left, const Node& right) {
    node.leftmost.validator = left.leftmost.validator;
    node.rightmost.validator = right.rightmost.validator;

    node.leftmost.deposit_index = left.leftmost.deposit_index;
    node.rightmost.deposit_index = right.rightmost.deposit_index;
}

void update_counted_data(Node& node, const Node& left, const Node& right) {
    Pubkey leftmost_pubkey = left.leftmost.validator.pubkey;
    Pubkey rightmost_pubkey = right.rightmost.validator.pubkey;

    bool leftmost_counted = left.leftmost.counted;
    leftmost_counted |= has_same_pubkey_and_is_counted(leftmost_pubkey, left.rightmost);
    leftmost_counted |= has_same_pubkey_and_is_counted(leftmost_pubkey, right.leftmost);
    leftmost_counted |= has_same_pubkey_and_is_counted(leftmost_counted, right.rightmost);

    bool rightmost_counted = right.rightmost.counted;
    rightmost_counted |= has_same_pubkey_and_is_counted(rightmost_pubkey, left.leftmost);
    rightmost_counted |= has_same_pubkey_and_is_counted(rightmost_pubkey, left.rightmost);
    rightmost_counted |= has_same_pubkey_and_is_counted(rightmost_pubkey, right.leftmost);

    node.leftmost.counted = leftmost_counted;
    node.rightmost.counted = rightmost_counted;
}

void account_for_double_counting(Node& node, const Node& left, const Node& right) {
    if (pubkeys_are_same_and_are_counted(left.rightmost, right.leftmost)) {
        node.accumulated_balance -= left.rightmost.validator.balance;

        node.validator_stats.non_activated_validators_count -= left.rightmost.validator.status_bits[NON_ACTIVATED_VALIDATORS_COUNT_BIT];
        node.validator_stats.active_validators_count -= left.rightmost.validator.status_bits[ACTIVE_VALIDATORS_COUNT_BIT];
        node.validator_stats.non_activated_validators_count -= left.rightmost.validator.status_bits[EXITED_VALIDATORS_COUNT_BIT];
    }
}

void accumulate_data(Node& node, const Node& left, const Node& right) {
    node.accumulated_balance = left.accumulated_balance + right.accumulated_balance;
    node.validator_stats = {
        .non_activated_validators_count =
            left.validator_stats.non_activated_validators_count +
            right.validator_stats.non_activated_validators_count,
        .active_validators_count =
            left.validator_stats.active_validators_count +
            right.validator_stats.active_validators_count,
        .exited_validators_count =
            left.validator_stats.exited_validators_count +
            right.validator_stats.exited_validators_count,
    };
}

Node compute_parent(const Node& left, const Node& right) {
    Node node;

    // ensure all the leaves are sorted by the tuple (pubkey, deposit_index) and deposit indices are unique
    assert(left.leftmost.validator.pubkey <= right.rightmost.validator.pubkey);
    assert(left.leftmost.deposit_index < right.rightmost.deposit_index);

    inherit_bounds_data_from_children(node, left, right);
    accumulate_data(node, left, right);
    account_for_double_counting(node, left, right);
    update_counted_data(node, left, right);

    return node;
}

auto build_binary_tree(const Vec<Node>& leaves) {
    Vec<Vec<Node>> tree;
    tree.push_back(leaves);

    while (tree.back().size() != 1) {
        const auto& lower_level = tree.back();

        Vec<Node> current_level;

        for (size_t i = 0; i < lower_level.size(); i += 2) {
            const auto& left = lower_level[i];
            const auto& right = lower_level[i + 1];
            current_level.push_back(compute_parent(left, right));
        }
        tree.push_back(current_level);
    }

    return tree;
}

template <size_t N>
void push_deposits_with_pubkey(
    Vec<Node>& deposits,
    Pubkey pubkey,
    u64 balance,
    const bool (&valid_signature_bitmask)[N],
    ValidatorEpochData epoch_data
) {
    for (size_t i = 0; i < N; ++i) {
        auto deposit_index = deposits.size();
        Node leaf = create_leaf_node(pubkey, deposit_index, balance, valid_signature_bitmask[i], epoch_data);
        deposits.push_back(leaf);
    }
}

int main() {
    Vec<Node> leaves;

    const auto active = ValidatorEpochData { CURRENT_EPOCH, CURRENT_EPOCH + 1 };
    const auto non_activated = ValidatorEpochData { CURRENT_EPOCH + 1, FAR_AWAY_EPOCH };
    const auto exited = ValidatorEpochData { CURRENT_EPOCH - 1, CURRENT_EPOCH };

    push_deposits_with_pubkey<5>(leaves, 1, 10, {0, 1, 1, 0, 1}, active);
    push_deposits_with_pubkey<1>(leaves, 2, 10, {1}, active);
    push_deposits_with_pubkey<2>(leaves, 3, 10, {1, 1}, active);
    push_deposits_with_pubkey<2>(leaves, 4, 10, {1, 1}, exited);
    push_deposits_with_pubkey<2>(leaves, 5, 10, {1, 1}, active);
    push_deposits_with_pubkey<1>(leaves, 6, 10, {1}, active);
    push_deposits_with_pubkey<3>(leaves, 7, 10, {1, 1, 1}, active);

    auto tree = build_binary_tree(leaves);
    for (const auto& level : tree) {
        for (const auto& node : level) {
            debug_print_node(node);
            std::cout << "\n";
        }
        std::cout << '\n';
    }
}

