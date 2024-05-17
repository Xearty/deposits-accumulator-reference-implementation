// clang-format off
#pragma once

#include <iostream>
#include <stdint.h>
#include <vector>
#include <bitset>

template <typename T>
using Vec = std::vector<T>;

template <size_t N>
using Bitset = std::bitset<N>;

using Pubkey = uint64_t;
using u64 = uint64_t;

enum VALIDATOR_STATUS_BITS {
    NON_ACTIVATED_VALIDATORS_COUNT_BIT,
    ACTIVE_VALIDATORS_COUNT_BIT,
    EXITED_VALIDATORS_COUNT_BIT,
};

struct ValidatorEpochData {
    u64 activation_epoch;
    u64 exit_epoch;
};

struct ValidatorData {
    Pubkey pubkey;
    u64 balance;
    Bitset<3> status_bits;
};

struct BoundsData {
    ValidatorData validator;
    u64 deposit_index;
    bool counted;
};

struct ValidatorStats {
    u64 non_activated_validators_count;
    u64 active_validators_count;
    u64 exited_validators_count;
};

struct AccumulatedData {
    u64 balance;
    ValidatorStats validator_stats;
};

struct Range {
    u64 start;
    u64 size;
};

struct Node {
    BoundsData leftmost;
    BoundsData rightmost;
    AccumulatedData accumulated;
    Range range;
};

static void debug_print_node(const Node& node) {
    std::cout << "data: {";
    std::cout << "pubkeys: {" << node.leftmost.validator.pubkey << ", " << node.rightmost.validator.pubkey << "}, ";
    std::cout << "deposit_index: {" << node.leftmost.deposit_index << ", " << node.rightmost.deposit_index << "}, ";
    std::cout << "balance: {" << node.leftmost.validator.balance << ", " << node.rightmost.validator.balance << "}, ";
    std::cout << "counted: {" << node.leftmost.counted << ", " << node.rightmost.counted;
    std::cout << "}, ";
    std::cout << "accumulated_balance: " << node.accumulated.balance << ", ";
    std::cout << "validator_stats: {" << node.accumulated.validator_stats.non_activated_validators_count;
    std::cout << ", " << node.accumulated.validator_stats.active_validators_count;
    std::cout << ", " << node.accumulated.validator_stats.exited_validators_count;
    std::cout << "}, ";
    std::cout << "range: {";
    std::cout << "start: " << node.range.start << ", ";
    std::cout << "size: " << node.range.size;
    std::cout << "}";
}

