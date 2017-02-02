#include "node.h"
#include "node_natives.h"
#include "v8.h"
#include "env.h"
#include "env-inl.h"

namespace node {

using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::NewStringType;
using v8::Object;
using v8::String;

template <typename T, size_t N, T P>
struct ExternalStringResource;

template <size_t N, const char (&P)[N]>
struct ExternalStringResource<const char[N], N, P>
    : public String::ExternalOneByteStringResource {
  const char* data() const override { return P; }
  size_t length() const override { return N; }
  void Dispose() override { /* Default calls `delete this`. */ }
};

template <size_t N, const uint16_t (&P)[N]>
struct ExternalStringResource<const uint16_t[N], N, P>
    : public String::ExternalStringResource {
  const uint16_t* data() const override { return P; }
  size_t length() const override { return N; }
  void Dispose() override { /* Default calls `delete this`. */ }
};

// id##_data is defined in node_natives.h.
#define V(id) \
  static ExternalStringResource<decltype(id##_data),                          \
                                arraysize(id##_data),                         \
                                id##_data> id##_external_data;
NODE_NATIVES_MAP(V)
#undef V

inline MaybeLocal<String>
ToExternal(Isolate* isolate, String::ExternalOneByteStringResource* that) {
  return String::NewExternalOneByte(isolate, that);
}

inline MaybeLocal<String>
ToExternal(Isolate* isolate, String::ExternalStringResource* that) {
  return String::NewExternalTwoByte(isolate, that);
}

Local<String> MainSource(Environment* env) {
  return ToExternal(env->isolate(),
                    &internal_bootstrap_node_external_data).ToLocalChecked();
}

void DefineJavaScript(Environment* env, Local<Object> target) {
  auto context = env->context();
#define V(id)                                                                 \
  do {                                                                        \
    auto key =                                                                \
        String::NewFromOneByte(                                               \
            env->isolate(), id##_name, NewStringType::kNormal,                \
            sizeof(id##_name)).ToLocalChecked();                              \
    auto value =                                                              \
        ToExternal(env->isolate(), &id##_external_data).ToLocalChecked();     \
    CHECK(target->Set(context, key, value).FromJust());                       \
  } while (0);
  NODE_NATIVES_MAP(V)
#undef V
}

}  // namespace node
