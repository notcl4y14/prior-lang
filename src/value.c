#include <value.h>

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

const char* ValueTypeNames[] = {
    [VT_NONE]    = "none",
    [VT_INT8]    = "i8",
    [VT_INT16]   = "i16",
    [VT_INT32]   = "i32",
    [VT_INT64]   = "i64",
    [VT_UINT8]   = "u8",
    [VT_UINT16]  = "u16",
    [VT_UINT32]  = "u32",
    [VT_UINT64]  = "u64",
    [VT_FLOAT32] = "f32",
    [VT_FLOAT64] = "f64",
};

Value cast_value(Value value, ValueType cast_type) {
    long double intermed_val = 0;

    switch (value.type) {
        case VT_INT8:    intermed_val = (long double) value.value.i8;  break;
        case VT_INT16:   intermed_val = (long double) value.value.i16; break;
        case VT_INT32:   intermed_val = (long double) value.value.i32; break;
        case VT_INT64:   intermed_val = (long double) value.value.i64; break;

        case VT_UINT8:   intermed_val = (long double) value.value.u8;  break;
        case VT_UINT16:  intermed_val = (long double) value.value.u16; break;
        case VT_UINT32:  intermed_val = (long double) value.value.u32; break;
        case VT_UINT64:  intermed_val = (long double) value.value.u64; break;

        case VT_FLOAT32: intermed_val = (long double) value.value.f32; break;
        case VT_FLOAT64: intermed_val = (long double) value.value.f64; break;

        default: assert(false); break;
    }

    Value result_value = { 0 };
    result_value.type = cast_type;

    switch (cast_type) {
        case VT_INT8:   result_value.value.i8  = (int8_t) intermed_val;   break;
        case VT_INT16:  result_value.value.i16 = (int16_t) intermed_val;  break;
        case VT_INT32:  result_value.value.i32 = (int32_t) intermed_val;  break;
        case VT_INT64:  result_value.value.i64 = (int64_t) intermed_val;  break;

        case VT_UINT8:  result_value.value.u8  = (uint8_t) intermed_val;  break;
        case VT_UINT16: result_value.value.u16 = (uint16_t) intermed_val; break;
        case VT_UINT32: result_value.value.u32 = (uint32_t) intermed_val; break;
        case VT_UINT64: result_value.value.u64 = (uint64_t) intermed_val; break;

        case VT_FLOAT32: result_value.value.f32 = (float) intermed_val;   break;
        case VT_FLOAT64: result_value.value.f64 = (double) intermed_val;  break;

        default: assert(false); break;
    }

    return result_value;
}
