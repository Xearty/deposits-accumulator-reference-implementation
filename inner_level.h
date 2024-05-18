// clang-format off
#pragma once

#include "types.h"
#include <assert.h>

static bool has_same_pubkey_and_is_counted(Pubkey pubkey, const BoundsData& data) {
    return pubkey == data.validator.pubkey && data.counted;
}

static bool pubkeys_are_same_and_are_counted(const BoundsData& first, const BoundsData& second) {
    bool pubkeys_are_same = first.validator.pubkey == second.validator.pubkey;
    bool both_are_counted = first.counted && second.counted;
    return pubkeys_are_same && both_are_counted;
}

static Tuple<bool, bool> calc_counted_data(const Node& left, const Node& right) {
    Pubkey leftmost_pubkey = left.leftmost.validator.pubkey;
    Pubkey rightmost_pubkey = right.rightmost.validator.pubkey;

    bool leftmost_counted = left.leftmost.counted
        || has_same_pubkey_and_is_counted(leftmost_pubkey, left.rightmost)
        || has_same_pubkey_and_is_counted(leftmost_pubkey, right.leftmost)
        || has_same_pubkey_and_is_counted(leftmost_pubkey, right.rightmost);

    bool rightmost_counted = right.rightmost.counted
        || has_same_pubkey_and_is_counted(rightmost_pubkey, left.leftmost)
        || has_same_pubkey_and_is_counted(rightmost_pubkey, left.rightmost)
        || has_same_pubkey_and_is_counted(rightmost_pubkey, right.leftmost);

    return { leftmost_counted, rightmost_counted };
}

static Tuple<BoundsData, BoundsData> inherit_bounds_data_from_children(const Node& left, const Node& right) {
    const auto [left_counted_data, right_counted_data] = calc_counted_data(left, right);

    BoundsData left_data = {
        .validator = left.leftmost.validator,
        .deposit_index = left.leftmost.deposit_index,
        .counted = left_counted_data,
        .is_fictional = left.leftmost.is_fictional,
    };

    BoundsData right_data = {
        .validator = right.rightmost.validator,
        .deposit_index = right.rightmost.deposit_index,
        .counted = left_counted_data,
        .is_fictional = right.rightmost.is_fictional,
    };

    return  { left_data, right_data };
}


static AccumulatedData account_for_double_counting(const AccumulatedData& accumulated_data, const Node& left, const Node& right) {
    if (!pubkeys_are_same_and_are_counted(left.rightmost, right.leftmost)) return accumulated_data;

    u64 balance = accumulated_data.balance - left.rightmost.validator.balance;

    u64 non_activated_validators_count = accumulated_data.validator_stats.non_activated_validators_count - left.rightmost.validator.status_bits[NON_ACTIVATED_VALIDATORS_COUNT_BIT];
    u64 active_validators_count = accumulated_data.validator_stats.active_validators_count - left.rightmost.validator.status_bits[ACTIVE_VALIDATORS_COUNT_BIT];
    u64 exited_validators_count = accumulated_data.validator_stats.exited_validators_count - left.rightmost.validator.status_bits[EXITED_VALIDATORS_COUNT_BIT];

    return AccumulatedData {
        .balance = balance,
        .deposits_count = accumulated_data.deposits_count,
        .validator_stats = {
            .non_activated_validators_count = non_activated_validators_count,
            .active_validators_count = active_validators_count,
            .exited_validators_count = exited_validators_count,
        },
    };
}

static ValidatorStats accumulate_validator_stats(const ValidatorStats& left, const ValidatorStats& right) {
    return {
        .non_activated_validators_count = left.non_activated_validators_count + right.non_activated_validators_count,
        .active_validators_count = left.active_validators_count + right.active_validators_count,
        .exited_validators_count = left.exited_validators_count + right.exited_validators_count,
    };
}

static AccumulatedData accumulate_data(const Node& left, const Node& right) {
    return {
        .balance = left.accumulated.balance + right.accumulated.balance,
        .deposits_count = left.accumulated.deposits_count + right.accumulated.deposits_count,
        .validator_stats = accumulate_validator_stats(left.accumulated.validator_stats, right.accumulated.validator_stats),
    };
}

static bool is_zero_proof(const Node& node) {
    return node.leftmost.is_fictional && node.rightmost.is_fictional;
}

static Node compute_parent(const Node& left, const Node& right) {
    // ensure all the leaves are sorted by the tuple (pubkey, deposit_index) and deposit indices are unique
    assert(left.rightmost.validator.pubkey <= right.leftmost.validator.pubkey);
    assert(left.rightmost.deposit_index < right.leftmost.deposit_index || is_zero_proof(right));

    const auto& [left_bounds_data, right_bounds_data] = inherit_bounds_data_from_children(left, right);
    AccumulatedData accumulated_data = account_for_double_counting(accumulate_data(left, right), left, right);

    return Node {
        .leftmost = left_bounds_data,
        .rightmost = right_bounds_data,
        .accumulated = accumulated_data,
    };
}

