#if !(defined(HELPER_CXX_COMPILER_IS_CLANG) && defined(HELPER_CXX_COMPILER_FRONTEND_VARIANT_IS_MSVC))

#if defined(__has_feature)
#  if __has_feature(address_sanitizer)
#ifdef __cplusplus
extern "C"
#endif
const char *__asan_default_options() {
  return "strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:detect_invalid_pointer_pairs=1:detect_leaks=1";
}
#  endif
#endif

#if defined(__has_feature)
#  if __has_feature(memory_sanitizer)
#ifdef __cplusplus
extern "C"
#endif
const char *__msan_default_options() {
  return "poison_heap_with_zeroes=1:poison_stack_with_zeroes=1:poison_in_malloc=1:poison_in_free=1:poison_in_dtor=1:report_umrs=1:wrap_signals=1:print_stats=1:halt_on_error=0";
}
#  endif
#endif

#endif
