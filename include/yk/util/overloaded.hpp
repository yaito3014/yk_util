#ifndef YK_VARIANT_OVERLOADED
#define YK_VARIANT_OVERLOADED

namespace yk {

template <class... Fs>
struct overloaded : public Fs... {
  using Fs::operator()...;
};

}  // namespace yk

#endif  // YK_VARIANT_OVERLOADED
