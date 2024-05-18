// clang-format off
#pragma once

#include "types.h"
#include "constants.h"

static Node create_deposit_leaf(Pubkey pubkey, u64 deposit_index, u64 balance, bool signature_is_valid, ValidatorEpochData epoch_data) {
    u64 non_activated_validators_count = CURRENT_EPOCH < epoch_data.activation_epoch;
    u64 active_validators_count = CURRENT_EPOCH >= epoch_data.activation_epoch && CURRENT_EPOCH < epoch_data.exit_epoch;
    u64 exited_validators_count = CURRENT_EPOCH >= epoch_data.exit_epoch;

    Bitset<3> validator_status_bits;
    validator_status_bits.set(NON_ACTIVATED_VALIDATORS_COUNT_BIT, non_activated_validators_count);
    validator_status_bits.set(ACTIVE_VALIDATORS_COUNT_BIT, active_validators_count);
    validator_status_bits.set(EXITED_VALIDATORS_COUNT_BIT, exited_validators_count);

    bool deposit_is_processed = ETH1_DATA_DEPOSIT_COUNT >= deposit_index;
    bool validator_is_definitely_in_chain = signature_is_valid && deposit_is_processed;

    BoundsData bounds_data = BoundsData {
        .validator = ValidatorData {
            .pubkey = pubkey,
            .balance = balance,
            .status_bits = validator_status_bits,
        },
        .deposit_index = deposit_index,
        .counted = validator_is_definitely_in_chain,
        .is_fictional = false,
    };

    AccumulatedData accumulated = validator_is_definitely_in_chain
        ? AccumulatedData {
            .balance = balance,
            .deposits_count = 1,
            .validator_stats = {
                .non_activated_validators_count = non_activated_validators_count,
                .active_validators_count = active_validators_count,
                .exited_validators_count = exited_validators_count,
            }
        }
        : AccumulatedData { .deposits_count = 1 };

    return Node {
        .leftmost = bounds_data,
        .rightmost = bounds_data,
        .accumulated = accumulated,
    };
}

static Node create_fictional_leaf() {
    BoundsData bounds_data = {
        .validator = { .pubkey = FIELD_ORDER - 1 },
        .is_fictional = true,
    };

    return Node {
        .leftmost = bounds_data,
        .rightmost = bounds_data,
        .accumulated = {},
    };
}

static Node create_leaf_node(
    Pubkey pubkey,
    u64 deposit_index,
    u64 balance,
    bool signature_is_valid,
    ValidatorEpochData epoch_data,
    bool is_fictional
) {
    return is_fictional
        ? create_fictional_leaf()
        : create_deposit_leaf(pubkey, deposit_index, balance, signature_is_valid, epoch_data);
}
