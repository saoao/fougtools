#include <codegenerator.h>

#include <iostream>

using namespace CodeGenerator;

const Item argument = "arg" + Counter();
const Item memberArgument = "m_arg" + Counter();
const Item argumentRef = "&arg" + Counter();
const Item argumentType  = "ARG" + Counter();
const Item constArgumentType  = "const ARG" + Counter();
const Item parameterType  = "PARAM" + Counter();

Group arguments(argument);
Group argumentTypes(argumentType); // expands to ",ARG1, ARG2, ..."
Group memberArgumentsNoPrefix(memberArgument); // expands to "arg1, arg2, ..."
Group typenameArgumentTypes("typename " + argumentType);
Group parameterTypesNoPrefix(parameterType); // expands to "PARAM1, PARAM2, ..."
Group types(parameterType + ", " + argumentType); // expands to ", PARAM1, ARG1, ..."

// expands to " ,typename PARAM1, typename ARG1, ..."
Group typenameTypes("typename " + parameterType + ", typename " + argumentType);

Group functionParameters(constArgumentType + " " + argumentRef);
Group initializers("m_" + argument + "(" + argument + ")");
Group classData(argumentType + " "  + "m_" + argument + ";");

void init()
{
  arguments.setPrefix(", ");
  argumentTypes.setPrefix(", ");
  typenameArgumentTypes.setPrefix(", ");
  functionParameters.setPrefix(", ");
  typenameTypes.setPrefix(", ");
  typenameArgumentTypes.setPrefix(", ");
  types.setPrefix(", ");

  initializers.setPrefix(", ");
  classData.setSeparator(" ");
  classData.setPrefix("  ");
}

Item Line(Item item)
{
  return item + "\n";
}

Item genericStoredFunctor()
{
  return
      Line("template<typename FUNCTOR>") +
      Line("class GenericStoredFunctor : public AbstractFunctor<typename FUNCTOR::result_type>") +
      Line("{") +
      Line("public:") +
      Line("  GenericStoredFunctor(FUNCTOR functor)") +
      Line("    : m_functor(functor)") +
      Line("  { }") +
      Line("") +
      Line("  typename AbstractFunctor<typename FUNCTOR::result_type>::ResultType execute()") +
      Line("  {") +
      Line("    return m_functor();") +
      Line("  }") +
      Line("") +
      Line("private:") +
      Line("  FUNCTOR m_functor;") +
      Line("};") +
      Line("") +
      Line("template<typename FUNCTOR>") +
      Line("AbstractFunctor<typename FUNCTOR::result_type>* newFunctor(FUNCTOR functor)") +
      Line("{") +
      Line("  return new GenericStoredFunctor<FUNCTOR>(functor);") +
      Line("}\n");
}

Item storedFunctors(Item className, Item functorType)
{
  return
      Line("template<typename T, typename FUNCTION_PTR" + typenameArgumentTypes + ">") +
      Line("struct " + className + Counter() + " : public AbstractFunctor<T>") +
      Line("{") +
      Line("  " + className + Counter() + "(" + functorType + " funcPtr" + functionParameters + ")") +
      Line("    : m_funcPtr(funcPtr)" + initializers + " { }") +
      Line("  T execute() { return m_funcPtr(" + memberArgumentsNoPrefix + "); }") +
      Line("  " + functorType + " m_funcPtr;") +
      Line(classData) +
      Line("};") +
      Line("");
}

Item memberFunctors(Item className,
                    Item constFunction,
                    Item objectArgument,
                    Item objectMember,
                    Item funcOp)
{
  return
      Line("template <typename T, typename CLASS"  + typenameTypes + ">") +
      Line("class " + className + Counter() + " : public AbstractFunctor<T>") +
      Line("{") +
      Line("public:")+
      Line("  " + className + Counter() + "(T (CLASS::*funcPtr)(" + parameterTypesNoPrefix  + ") " + constFunction + ", " + objectArgument + functionParameters + ")") +
      Line("    : m_funcPtr(funcPtr), m_object(object)" + initializers + " { }" ) +
      Line("")+
      Line("  T execute()")+
      Line("  {")+
      Line("    return (m_object" + funcOp + "*m_funcPtr)(" + memberArgumentsNoPrefix +  ");")+
      Line("  }")+
      Line("private:")+
      Line("  T (CLASS::*m_funcPtr)(" + parameterTypesNoPrefix  + ") " + constFunction + ";")+
      Line("  " + objectMember + ";") +
      Line(   classData) +
      Line("};") +
      Line("");
}

Item newFunctorFunctions(int repeatCount)
{
  Item functionPointerType = "T (*)(" + parameterTypesNoPrefix + ")";
  Item functionPointerParameter = "T (*functionPointer)(" + parameterTypesNoPrefix + ")";

  // plain functions
  Repeater function =
      Line("template<typename T" + typenameTypes + ">") +
      Line ("AbstractFunctor<T>* newFunctor(" + functionPointerParameter + functionParameters + ")")  +
      Line("{") +
      Line("  return new StoredFunctor" + Counter() + "<T, " + functionPointerType + argumentTypes + ">" +
           "(functionPointer" + arguments + ");") +
      Line("}\n");
  function.setRepeatCount(repeatCount);

  // member functions by value
  Repeater memberFunction =
      Line ("template <typename T, typename CLASS" + typenameTypes + ">") +
      Line ("AbstractFunctor<T>* newFunctor(const CLASS& object, T (CLASS::*funcPtr)(" + parameterTypesNoPrefix  + ")" + functionParameters + ")") +
      Line("{") +
      Line("  return new StoredMemberFunctor" + Counter() + "<T, CLASS" + types + ">" +
           "(funcPtr, object" + arguments + ");") +
      Line("}\n");
  memberFunction.setRepeatCount(repeatCount);

  // const member functions by value
  Repeater constMemberFunction =
      Line ("template <typename T, typename CLASS" + typenameTypes + ">") +
      Line ("AbstractFunctor<T>* newFunctor(const CLASS& object, T (CLASS::*funcPtr)(" + parameterTypesNoPrefix  + ") const" + functionParameters + ")") +
      Line("{") +
      Line("  return new StoredConstMemberFunctor" + Counter() + "<T, CLASS" + types + ">" +
           "(funcPtr, object" + arguments + ");") +
      Line("}\n");
  constMemberFunction.setRepeatCount(repeatCount);

  // member functions by class pointer
  Repeater memberFunctionPtr =
      Line ("template <typename T, typename CLASS" + typenameTypes + ">") +
      Line ("AbstractFunctor<T>* newFunctor(CLASS* object, T (CLASS::*funcPtr)(" + parameterTypesNoPrefix  + ")" + functionParameters + ")") +
      Line("{") +
      Line("  return new StoredMemberFunctorPtr" + Counter() + "<T, CLASS" + types + ">" +
           "(funcPtr, object" + arguments + ");") +
      Line("}\n");
  memberFunctionPtr.setRepeatCount(repeatCount);

  // const member functions by class pointer
  Repeater constMemberFunctionPtr =
      Line ("template <typename T, typename CLASS" + typenameTypes + ">") +
      Line ("AbstractFunctor<T>* newFunctor(const CLASS* object, T (CLASS::*funcPtr)(" + parameterTypesNoPrefix  + ") const" + functionParameters + ")") +
      Line("{") +
      Line("  return new StoredConstMemberFunctorPtr" + Counter() + "<T, CLASS" + types + ">" +
           "(funcPtr, object" + arguments + ");") +
      Line("}\n");
  constMemberFunctionPtr.setRepeatCount(repeatCount);

  return
      function + Line("") +
      memberFunction + Line("") +
      constMemberFunction + Line("") +
      memberFunctionPtr + Line("") +
      constMemberFunctionPtr + Line("");
}

int main(int argc, char** argv)
{
  init();

  Repeater dataStructs =
      storedFunctors(Item("StoredFunctor"), Item("FUNCTION_PTR"))
      + storedFunctors(Item("StoredFunctorPtr"), Item("FUNCTION_PTR*"))
      + memberFunctors(Item("StoredMemberFunctor"), Item(""), Item("const CLASS& object"), Item("CLASS m_object"), Item("."))
      + memberFunctors(Item("StoredConstMemberFunctor"), Item("const"), Item("const CLASS& object"), Item("const CLASS m_object"), Item("."))
      + memberFunctors(Item("StoredMemberFunctorPtr"), Item(""), Item("CLASS* object"), Item("CLASS* m_object"), Item("->"))
      + memberFunctors(Item("StoredConstMemberFunctorPtr"), Item("const"), Item("CLASS const* object"), Item("CLASS const* m_object"), Item("->"));
  dataStructs.setRepeatCount(4);

  Item newFunctors = newFunctorFunctions(4);

  Item rootItem =
      Line("/****************************************************************************") +
      Line("**") +
      Line("**  FougTools") +
      Line("**  Copyright FougSys (1 Mar. 2011)") +
      Line("**  contact@fougsys.fr") +
      Line("**") +
      Line("** This software is a computer program whose purpose is to provide utility") +
      Line("** tools for the C++ language and the Qt toolkit.") +
      Line("**") +
      Line("** This software is governed by the CeCILL-C license under French law and") +
      Line("** abiding by the rules of distribution of free software.  You can  use,") +
      Line("** modify and/ or redistribute the software under the terms of the CeCILL-C") +
      Line("** license as circulated by CEA, CNRS and INRIA at the following URL") +
      Line("** \"http://www.cecill.info\".") +
      Line("**") +
      Line("** As a counterpart to the access to the source code and  rights to copy,") +
      Line("** modify and redistribute granted by the license, users are provided only") +
      Line("** with a limited warranty  and the software's author,  the holder of the") +
      Line("** economic rights,  and the successive licensors  have only  limited") +
      Line("** liability.") +
      Line("**") +
      Line("** In this respect, the user's attention is drawn to the risks associated") +
      Line("** with loading,  using,  modifying and/or developing or reproducing the") +
      Line("** software by the user in light of its specific status of free software,") +
      Line("** that may mean  that it is complicated to manipulate,  and  that  also") +
      Line("** therefore means  that it is reserved for developers  and  experienced") +
      Line("** professionals having in-depth computer knowledge. Users are therefore") +
      Line("** encouraged to load and test the software's suitability as regards their") +
      Line("** requirements in conditions enabling the security of their systems and/or") +
      Line("** data to be ensured and,  more generally, to use and operate it in the") +
      Line("** same conditions as regards security.") +
      Line("**") +
      Line("** The fact that you are presently reading this means that you have had") +
      Line("** knowledge of the CeCILL-C license and that you accept its terms.") +
      Line("**") +
      Line("****************************************************************************/") +
      Line("") +
      Line("// Generated code, do not edit!") +
      Line("#ifndef CPPTOOLS_NEW_FUNCTOR_H") +
      Line("#define CPPTOOLS_NEW_FUNCTOR_H") +
      Line("") +
      Line("#include \"abstract_functor.h\"") +
      Line("#include \"generic_stored_functor.h\"") +
      Line("") +
      Line("namespace cpp {") +
      Line("") +
      dataStructs +
      newFunctors +
      Line("} // namespace cpp") +
      Line("") +
      Line("#endif // CPPTOOLS_NEW_FUNCTOR_H") +
      Line("");

  std::cout << rootItem.generate().constData() << std::endl;

  return 0;
}