#ifndef __PARAMETER_H__
#define __PARAMETER_H__

#include <string>
#include <iostream>
#include <typeindex>

class ParameterBase
{
public:
  const std::string m_name;
  const std::string m_desc;
  const std::type_index m_type;

  ParameterBase(
    const std::string& name, 
    const std::string& desc,
    const std::type_index& type
  )
    : m_name(name),
      m_desc(desc),
      m_type(type)
  {}
};


template <typename T>
class Parameter
  : public ParameterBase
{
  // friend std::ostream& operator<< <T> (std::ostream& os, const Parameter<T>& param);

public:
  Parameter(
    const std::string& name,
    T val,
    const std::string& desc
  )
    : ParameterBase(name, desc, typeid(T)),
      m_val(val)
  {}

  operator T () { return m_val; }

  std::string name() { return m_name; }
  void set_val(T val) { m_val = val; }
  T get_val() { return m_val; }
  
public:
  T m_val;
};

template <typename T>
std::ostream& operator << (std::ostream& os, const Parameter<T>& param)
{
os << "Paramter: { name: " << param.m_name 
    << ", val: " << param.m_val
    << ", type: " << param.m_type.name()
    << ", desc: " << param.m_desc
    << " }";
  return os;
}

#endif  /* __PARAMETER_H__ */
