// clang-format off
#pragma once

#include "types.h"
#include "first_level.h"
#include "inner_level.h"

static Vec<Vec<Node>> build_binary_tree(const Vec<Node>& leaves) {
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
        Node leaf = create_leaf_node(pubkey, deposit_index, balance, valid_signature_bitmask[i], epoch_data, false);
        deposits.push_back(leaf);
    }
}

static void push_fictional_deposits(Vec<Node>& deposits, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        auto deposit_index = deposits.size();
        Node leaf = create_leaf_node(0, 0, 0, 0, {}, true);
        deposits.push_back(leaf);
    }
}
