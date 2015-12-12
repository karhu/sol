#pragma once

//namespace sol { namespace meta {

namespace internal
{
#if defined(__GNUC__)
  static const unsigned int TYPE_NAME_FRONT_SIZE = sizeof("static const char* internal::get_type_name_helper<T>::get() [with T = ") - 1u;
  static const unsigned int TYPE_NAME_BACK_SIZE = sizeof("]") - 1u;

  template <typename T>
  struct get_type_name_helper
  {
    static const char* get(void)
    {
      static const size_t size = sizeof(__PRETTY_FUNCTION__ ) - TYPE_NAME_FRONT_SIZE - TYPE_NAME_BACK_SIZE;
      static char type_name[size] = {};
      memcpy(type_name, __PRETTY_FUNCTION__ + TYPE_NAME_FRONT_SIZE, size - 1u);

      return type_name;
    }
  };
#else
    static const unsigned int TYPE_NAME_FRONT_SIZE = sizeof("internal::GetTypeNameHelper<") - 1u;
    static const unsigned int TYPE_NAME_BACK_SIZE = sizeof(">::GetTypeName") - 1u;

    template <typename T>
    struct get_type_name_helper
    {
      static const char* get(void)
      {
        static const size_t size = sizeof(__FUNCTION__) - TYPE_NAME_FRONT_SIZE - TYPE_NAME_BACK_SIZE;
        static char type_name[size] = {};
        memcpy(type_name, __FUNCTION__ + TYPE_NAME_FRONT_SIZE, size - 1u);

        return type_name;
      }
    };
#endif
}


template <typename T>
const char* get_type_name(void)
{
  return internal::get_type_name_helper<T>::get();
}


//}}
