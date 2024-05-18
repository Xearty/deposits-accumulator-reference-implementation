// clang-format off

#include "util.h"

int main() {
    const auto active = ValidatorEpochData { CURRENT_EPOCH, CURRENT_EPOCH + 1 };
    const auto non_activated = ValidatorEpochData { CURRENT_EPOCH + 1, FAR_AWAY_EPOCH };
    const auto exited = ValidatorEpochData { CURRENT_EPOCH - 1, CURRENT_EPOCH };

    Vec<Node> leaves;
    push_deposits_with_pubkey(leaves, 1, 10, {0, 1, 1, 0, 1}, active);
    push_deposits_with_pubkey(leaves, 2, 10, {1},             active);
    push_deposits_with_pubkey(leaves, 3, 10, {1, 1},          active);
    push_deposits_with_pubkey(leaves, 4, 10, {1, 1},          exited);
    push_deposits_with_pubkey(leaves, 5, 10, {1, 1},          active);
    push_deposits_with_pubkey(leaves, 6, 10, {1},             active);
    push_deposits_with_pubkey(leaves, 7, 10, {1, 1, 1},       non_activated);
    push_deposits_with_pubkey(leaves, 8, 10, {0},             non_activated);
    push_fictional_deposits(leaves, 15);

    auto tree = build_binary_tree(leaves);
    for (const auto& level : tree) {
        for (const auto& node : level) {
            debug_print_node(node);
            std::cout << "\n";
        }
        std::cout << '\n';
    }
}

