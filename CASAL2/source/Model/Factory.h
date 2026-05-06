/**
 * @file Factory.h
 * @author Scott Rasmussen (scott.rasmussen@zaita.com)
 * @github https://github.com/Zaita
 * @date 8/10/2015
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/�2015 - www.niwa.co.nz
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifndef FACTORY_H_
#define FACTORY_H_

// headers
#include <memory>
#include <string>

#include "../BaseClasses/Object.h"
#include "../Utilities/PartitionType.h"

using std::shared_ptr;

// namespaces
namespace niwa {

using std::string;
class Model;
namespace base {
class Object;
}

/**
 * Class definition
 */
class Factory {
  friend class Model;

public:
  // methods
  base::Object*            CreateObject(const string& object_type, const string& sub_type, PartitionType partition_type = PartitionType::kInvalid);
  static shared_ptr<Model> Create(const string& object_type, const string& sub_type);

private:
  // methods
  Factory(shared_ptr<Model> model);
  virtual ~Factory() = default;
  shared_ptr<Model> model() const { return base::Object::LockWeakPtr(model_, "Factory"); }

  // members
  weak_ptr<Model> model_;
};

} /* namespace niwa */
#endif /* FACTORY_H_ */
