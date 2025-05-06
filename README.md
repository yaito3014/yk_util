[![CI](https://github.com/yaito3014/yk_util/actions/workflows/run_test.yml/badge.svg?branch=main)](https://github.com/yaito3014/yk_util/actions/workflows/run_test.yml)

# yk_util

## Overview
C++ utilities for C++23 or higher.

## Prerequirements
- C++ compiler that supports C++23
- Boost (optional)
  - Boost.Stacktrace (required for `yk::throwt` or `yk::printt`)
  - Boost.Test (required for testing)
  - Boost.ContainerHash (there are adaptors for `boost::hash` or `boost::hash_combine`)
  - Boost.Lockfree (there are adaptors for `boost::lockfree::queue`)
  - Boost.Range (there are adaptors for `boost::iterator_range`)
  - Boost.Variant (there are adaptors for `boost::variant`)
- [sg14::inplace_vector](https://github.com/Quuxplusone/SG14) (optional; there are helper header for `sg14::inplace_vector`)
