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
    NON_ACTIVATED_VALIDATORS_COUNT,
    ACTIVE_VALIDATORS_COUNT,
    EXITED_VALIDATORS_COUNT,
};

struct ValidatorEpochData {
    u64 activation_epoch;
    u64 exit_epoch;
};

struct ValidatorStats {
    u64 non_activated_validators_count = 0;
    u64 active_validators_count = 0;
    u64 exited_validators_count = 0;
};

struct ValidatorData {
    Pubkey pubkey;
    u64 balance;
    Bitset<3> status_bits;
};

struct Data {
    ValidatorData validator;
    u64 deposit_index;
    bool counted;
};

struct Node {
    Data leftmost;
    Data rightmost;
    u64 accumulated_balance;
    ValidatorStats validator_stats;
};

static void debug_print_node(const Node& node) {
    std::cout << "data: {";
    std::cout << "pubkeys: {" << node.leftmost.validator.pubkey << ", " << node.rightmost.validator.pubkey << "}, ";
    std::cout << "deposit_index: {" << node.leftmost.deposit_index << ", " << node.rightmost.deposit_index << "}, ";
    std::cout << "balance: {" << node.leftmost.validator.balance << ", " << node.rightmost.validator.balance << "}, ";
    std::cout << "counted: {" << node.leftmost.counted << ", " << node.rightmost.counted << "}, ";
    std::cout << "}, ";
    std::cout << "accumulated_balance: " << node.accumulated_balance << ", ";
    std::cout << "validator_stats: {" << node.validator_stats.non_activated_validators_count;
    std::cout << ", " << node.validator_stats.active_validators_count;
    std::cout << ", " << node.validator_stats.exited_validators_count;
    std::cout << "}";
}

