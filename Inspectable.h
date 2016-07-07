//////////////////////////////////////////////////////////////////////////////////////////
// Inspectable.h (version 0.1, July 2016)
//
//  A public domain single header file module for inspecting and modifying data
//  functionally. C++11 or newer required.
//
//  AUTHOR
//    Jared Thomson (@xoorath)
//
//  STREET CRED
//    An inspired implementation of Twisted Oaks Lens'. 
//    https://github.com/twistedoakstudios/tounityutils/tree/master/assets/lenses
//
//    Also inspired Sean Barrett's stb.
//    https://github.com/nothings
//
//  LICENSE
//
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
//////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <algorithm>
#include <climits>
#include <functional>

//////////////////////////////////////////////////////////////////////////////////////////
// Customization
//////////////////////////////////////////////////////////////////////////////////////////
// You can manually define xoins_list, xoins_list_add and xoins_list_remove before
// including this file to determine which std::vector compatible container and methods
// are used.
//
// Note: currently the container must also contain .begin() and .end() to be compatible
// with find. We also pass an itterator found this way to the erase method.
//
// Note: until noted otherwise: this functionality is completely untested. Let me know if
// you end up using it, or want to be a use case.
//
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef xoins_list
#include <vector>
#define xoins_list_internal         1
#define xoins_list                  std::vector
#endif // xoins_list

#ifndef xoins_list_add
#define xoins_list_add_internal     1
#define xoins_list_add              push_back
#endif // xoins_list_add

#ifndef xoins_list_erase
#define xoins_list_erase_internal   1
#define xoins_list_erase            erase
#endif // xoins_list_erase

//////////////////////////////////////////////////////////////////////////////////////////
// InspectableTransform
//////////////////////////////////////////////////////////////////////////////////////////
// An inspectable transform stores the function which modifies an inspectable value. Its
// function will be called to modify an inspectable value if the transform is enabled
// (m_Enabled) and its function (m_Function) has a valid target. It will be called in
// descending order of priority (m_Priority) relative to all the other inspectable
// transforms attached to a given inspectable value.
//
// To attach an inspectable transform to an inspectable value see:
// Inspectable<T>::AddTransform
//
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class InspectableTransformation {
public:
  typedef std::function<void(T&)> TTransformFunc;

  InspectableTransformation();
  InspectableTransformation(TTransformFunc func,
                            int priority = 0,
                            bool enabled = true);

  void Set(TTransformFunc func,
           int priority = 0,
           bool enabled = true);

  // note: there's no reliable and performant way to automatically call ForceUpdate
  // from InspectableTransformation's Enable/Disable methods. You will need to call
  // ForceUpdate yourself after enabling/disabling this transformation. An alternative
  // is to use an InspectableScopedTransformation which can ForceUpdate on En/Disable
  void Enable();
  void Disable();
  bool IsEnabled() const;
  int GetPriority() const;

  const TTransformFunc & GetTransformFunc() const; // Get the attached transformation
  void operator()(T& input); // Call the attached transformation


  const int MaxPriority = INT_MAX;
  const int MinPriority = INT_MIN + 1;
  const int InvalidPriority = INT_MIN;

private:
  int m_Priority;
  bool m_Enabled;
  TTransformFunc m_Function;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Inspectable
//////////////////////////////////////////////////////////////////////////////////////////
// An inspectable represents an identity value (m_Identity) modified by a set of functions
// (m_Transformations).
//
// An example use case could be a player speed with an identity of 5.
// You could then have a MudTrap InspectableTransform which subtracts 1 from the player
// speed. You could also have a PowerPill InspectableTransform which multiplies the player
// speed by 1.5. With only a mud trap, the player speed would become 4. With only a power
// pill, the player speed would become 7.5. Now if you added both (MudTrap then PowerPill)
// the speed would become 6 (applying the MudTrap then the PowerPill). If you wanted the
// multiplication to always happen first, you could increase the priority of the PowerPill
// causing the resulting value to become 6.5.
//
// Notice in the above example how a player with a speed value doesn't need to be aware of
// every type of trap or boost in the game. The traps and boosts also don't need to know
// about each other. You can have shared control over a value without adding a dependancy
// chain and allow you as a designer to change the priority of effects without digging
// through if statements to handle complex use cases.
//
// Inspectables also take the attitude of "performance first" to keep things from getting
// unwieldly. An inspectable will not be forced to update on every change unless you say
// so. You can also call ForceUpdate manually when you need to apply changes or provide
// GetValue with an 'andUpdate' bool to instruct it to update the value before retrieving
// its cached value.
//
// You can subscribe to changes in the resulting inspectable value with AddOnValueChanged
// Or just subscribe to changes in the identity with AddOnIdentityChanged.
//
// For your convenience there are also scoped values that can handle the lifecycle of
// callbacks and transformations. See ScopedInspectableTransform,
// InspectableScopedValueChangedFunc and InspectableScopedIdentityChangedFunc
//
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class Inspectable {
  typedef std::function<void(T&)> TTransformFunc; // must reflect TTransformFunc in InspectableTransformation
  typedef InspectableTransformation<T> TTransform;
public:
  typedef std::function<void(Inspectable<T>*, const T& /*lastValue*/, const T& /*newValue*/)> TValueChangedFunc;

  Inspectable();
  Inspectable(T identity);

  Inspectable<T>&   AddTransformation(TTransform& outTransformation,
                                      TTransformFunc func,
                                      int priority = 0,
                                      bool enabled = true,
                                      bool andUpdate = false);
  Inspectable<T>&   AddTransformation(        TTransform* transformation, bool andUpdate = false);
  Inspectable<T>&   AddTransformationUnique(  TTransform* transformation, bool andUpdate = false);
  void              RemoveTransformation(     TTransform* transformation, bool andUpdate = false);
  bool              ContainsTransformation(   TTransform* transformation) const;

  Inspectable<T>&   AddOnIdentityChanged(       TValueChangedFunc* f);
  Inspectable<T>&   AddOnIdentityChangedUnique( TValueChangedFunc* f);
  void              RemoveOnIdentityChanged(    TValueChangedFunc* f);
  bool              ContainsOnIdentityChanged(  TValueChangedFunc* f) const;

  Inspectable<T>&   AddOnValueChanged(        TValueChangedFunc* f);
  Inspectable<T>&   AddOnValueChangedUnique(  TValueChangedFunc* f);
  void              RemoveOnValueChanged(     TValueChangedFunc* f);
  bool              ContainsOnValueChanged(   TValueChangedFunc* f) const;

  void              ForceUpdate();

  void              SetIdentity(const T& value, bool andUpdate = false);
  const T&          GetValue(bool andUpdate = false);

private:
  void              SortTransformations();

  T                               m_Identity;
  T                               m_LastValue;
  xoins_list<TTransform*>         m_Transformations;
  xoins_list<TValueChangedFunc*>  m_IdentityChanged;
  xoins_list<TValueChangedFunc*>  m_ValueChanged;
};

//////////////////////////////////////////////////////////////////////////////////////////
// InspectableScopedTransformation
//////////////////////////////////////////////////////////////////////////////////////////
// This class wraps an InspectableTransform and takes an Inspectable in its constructor.
// Assuming a valid inspectable and function are provided, it will create an
// IinspectableTransform for you, and add it to the inspectable. On destruction of this
// object, it will remove the InspectableTransform from the Inspectable automatically.
//
// Optionally the inspectable can be told to update on attach, as well as on detatch.
//
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class InspectableScopedTransformation {
  typedef std::function<void(T&)> TTransformFunc; // must reflect TTransformFunc in InspectableTransformation

public:
  InspectableScopedTransformation(Inspectable<T>* inspectable = nullptr,
                                  bool updateOnDestroy = false);
  InspectableScopedTransformation(Inspectable<T>* inspectable,
                                  TTransformFunc func,
                                  int priority = 0,
                                  bool enabled = true,
                                  bool andUpdate = false,
                                  bool updateOnDestroy = false);

  ~InspectableScopedTransformation();

  void Set(TTransformFunc func,
           int priority = 0,
           bool enabled = true,
           bool andUpdate = false);

  void SetUpdateOnDestroy(bool updateOnDestroy);

  void Enable(bool andUpdate = false);
  void Disable(bool andUpdate = false);

  bool IsEnabled() const;
  int GetPriority() const;

  const TTransformFunc & GetTransformFunc() const; // Get the attached transformation
  void operator()(T& input); // Call the attached transformation

private:
  Inspectable<T>*               m_Inspectable;
  InspectableTransformation<T>  m_Transformation;
  bool                          m_UpdateOnDestroy;
};

//////////////////////////////////////////////////////////////////////////////////////////
// InspectableScopedValueChangedFunc
//////////////////////////////////////////////////////////////////////////////////////////
// This class wraps an std::function matching the OnValueChanged method signature in
// Inspectable. It will add a given std::function from the inspectable on
// construction (or Set) and remove it on destruction.
//
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class InspectableScopedValueChangedFunc
{
public:
  typedef std::function<void(Inspectable<T>*, const T& /*lastValue*/, const T& /*newValue*/)> TValueChangedFunc;

  InspectableScopedValueChangedFunc();
  InspectableScopedValueChangedFunc(Inspectable<T>* inspectable, TValueChangedFunc func);
  ~InspectableScopedValueChangedFunc();

  void Set(Inspectable<T>* inspectable, TValueChangedFunc func);
  void SetInspectable(Inspectable<T>* inspectable);
  void SetFunc(TValueChangedFunc func);

private:
  Inspectable<T>* m_Inspectable;
  TValueChangedFunc m_OnValueChanged;
};

//////////////////////////////////////////////////////////////////////////////////////////
// InspectableScopedIdentityChangedFunc
//////////////////////////////////////////////////////////////////////////////////////////
// This class wraps an std::function matching the OnValueChanged method signature in
// Inspectable. It will add a given std::function from the inspectable on
// construction (or Set) and remove it on destruction.
//
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class InspectableScopedIdentityChangedFunc
{
public:
  typedef std::function<void(Inspectable<T>*, const T& /*lastValue*/, const T& /*newValue*/)> TValueChangedFunc;

  InspectableScopedIdentityChangedFunc();
  InspectableScopedIdentityChangedFunc(Inspectable<T>* inspectable, TValueChangedFunc func);
  ~InspectableScopedIdentityChangedFunc();

  void Set(Inspectable<T>* inspectable, TValueChangedFunc func);
  void SetInspectable(Inspectable<T>* inspectable);
  void SetFunc(TValueChangedFunc func);

private:
  Inspectable<T>* m_Inspectable;
  TValueChangedFunc m_OnValueChanged;
};

//////////////////////////////////////////////////////////////////////////////////////////
// INTERNAL
//////////////////////////////////////////////////////////////////////////////////////////
// Below are a series of internal implementation details. The only API to note here is
// a typedef for basic types, specifying the template on each of the classes in this file.
//
// They take the form of: InspectableF (float) InspectableB (bool) InspectableU (unsigned)
// and so on.
//
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// InspectableTransform
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
InspectableTransformation<T>::InspectableTransformation() {
}

template<typename T>
InspectableTransformation<T>::InspectableTransformation(TTransformFunc func,
                                                        int priority,
                                                        bool enabled)
: m_Function(func),
m_Priority(priority),
m_Enabled(enabled)
{
}

template<typename T>
void InspectableTransformation<T>::Set(TTransformFunc func, int priority, bool enabled) {
  m_Function = func;
  m_Priority = priority;
  m_Enabled = enabled;
}

template<typename T>
void InspectableTransformation<T>::Enable() {
  m_Enabled = true;
}

template<typename T>
void InspectableTransformation<T>::Disable() {
  m_Enabled = false;
}

template<typename T>
bool InspectableTransformation<T>::IsEnabled() const {
  return m_Enabled;
}

template <typename T>
int InspectableTransformation<T>::GetPriority() const {
  return m_Priority;
}

template<typename T>
void InspectableTransformation<T>::operator ()(T& input) {
  m_Function(input);
}

template<typename T>
const std::function<void(T&)>& InspectableTransformation<T>::GetTransformFunc() const {
  return m_Function;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Inspectable
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
Inspectable<T>::Inspectable() {}

template<typename T>
Inspectable<T>::Inspectable(T identity) : m_Identity(identity), m_LastValue(identity) {
}

template<typename T>
Inspectable<T>& Inspectable<T>::AddTransformation(TTransform& outTransformation,
                                                  TTransformFunc func,
                                                  int priority,
                                                  bool enabled,
                                                  bool andUpdate) {
  outTransformation.Set(func, priority, enabled);
  return AddTransformation(&outTransformation, andUpdate);
}

template<typename T>
Inspectable<T>& Inspectable<T>::AddTransformation(TTransform* transformation,
                                                  bool andUpdate) {
  if(transformation == nullptr) // we don't store null transformations.
    return *this;
  m_Transformations.xoins_list_add(transformation);
  SortTransformations();
  if(andUpdate)
    ForceUpdate();
  return *this;
}

template<typename T>
Inspectable<T>& Inspectable<T>::AddTransformationUnique(TTransform* transformation,
                                                        bool andUpdate) {
  if(transformation == nullptr) // we don't store null transformations.
    return *this;
  auto found = std::find(m_Transformations.begin(), m_Transformations.end(), transformation);
  if(found == m_Transformations.end()) {
    m_Transformations.xoins_list_add(transformation);
    SortTransformations();
    if(andUpdate) // only update when a transformation was actually added.
      ForceUpdate();
  }
  return *this;
}

template<typename T>
void Inspectable<T>::RemoveTransformation(TTransform* transformation,
                                          bool andUpdate) {
  if(transformation == nullptr) // we don't store null transformations.
    return;
  auto found = std::find(m_Transformations.begin(), m_Transformations.end(), transformation);
  if(found != m_Transformations.end()) {
    m_Transformations.xoins_list_erase(found);
    if(andUpdate) // only update when a transformation was actually removed.
      ForceUpdate();
  }
}

template<typename T>
bool Inspectable<T>::ContainsTransformation(TTransform* transformation) const {
  if(!transformation) // we don't store null transformations.
    return false;
  auto found = std::find(m_Transformations.begin(), m_Transformations.end(), transformation);
  return found != m_Transformations.end();
}

template<typename T>
Inspectable<T>& Inspectable<T>::AddOnIdentityChanged(TValueChangedFunc* f) {
  if(f && *f) // we don't store null or targetless functions
    m_IdentityChanged.xoins_list_add(f);
  return *this;
}

template<typename T>
Inspectable<T>& Inspectable<T>::AddOnIdentityChangedUnique(TValueChangedFunc* f) {
  if(f && *f) { // we don't store null or targetless functions
    auto found = std::find(m_IdentityChanged.begin(), m_IdentityChanged.end(), f);
    if(found == m_IdentityChanged.end())
      m_IdentityChanged.xoins_list_add(f);
  }
  return *this;
}

template<typename T>
void Inspectable<T>::RemoveOnIdentityChanged(TValueChangedFunc* f) {
  if(f && *f) { // we don't store null or targetless functions
    auto found = std::find(m_IdentityChanged.begin(), m_IdentityChanged.end(), f);
    if(found != m_IdentityChanged.end())
      m_IdentityChanged.xoins_list_erase(found);
  }
}

template<typename T>
bool Inspectable<T>::ContainsOnIdentityChanged(TValueChangedFunc* f) const {
  if(f && *f) { // we don't store null or targetless functions
    auto found = std::find(m_IdentityChanged.begin(), m_IdentityChanged.end(), f);
    return found != m_IdentityChanged.end();
  }
  return false;
}

template<typename T>
Inspectable<T>& Inspectable<T>::AddOnValueChanged(TValueChangedFunc* f) {
  if(f && *f) // we don't store null or targetless functions
    m_ValueChanged.xoins_list_add(f);
  return *this;
}

template<typename T>
Inspectable<T>& Inspectable<T>::AddOnValueChangedUnique(TValueChangedFunc* f) {
  if(f && *f) { // we don't store null or targetless functions
    auto found = std::find(m_ValueChanged.begin(), m_ValueChanged.end(), f);
    if(found == m_ValueChanged.end())
      m_ValueChanged.xoins_list_add(f);
  }
  return *this;
}

template<typename T>
void Inspectable<T>::RemoveOnValueChanged(TValueChangedFunc* f) {
  if(f && *f) { // we don't store null or targetless functions
    auto found = std::find(m_ValueChanged.begin(), m_ValueChanged.end(), f);
    if(found != m_ValueChanged.end())
      m_ValueChanged.xoins_list_erase(found);
  }
}

template<typename T>
bool Inspectable<T>::ContainsOnValueChanged(TValueChangedFunc* f) const {
  if(f && *f) { // we don't store null or targetless functions
    auto found = std::find(m_ValueChanged.begin(), m_ValueChanged.end(), f);
    return found != m_ValueChanged.end();
  }
  return false;
}

template<typename T>
void Inspectable<T>::ForceUpdate()
{
  T value = m_Identity;
  // do a copy here so our m_LastValue can be correct for the duration of all callbacks.
  T lastValue = m_LastValue;

  for(auto transform : m_Transformations)
    // note: having no target is supported, since it can be set after adding
    // the transform to the inspectable.
    if(transform->IsEnabled() && (*transform).GetTransformFunc())
      (*transform)(value);

  m_LastValue = value;
  if(lastValue != value) {
    // having no target here is not supported since it could not be updated later.
    // because of that, no check for unset target is required here (it's done when adding)
    for(auto func : m_ValueChanged)
      (*func)(this, lastValue, value);
  }
}

template<typename T>
void Inspectable<T>::SetIdentity(const T& value, bool andUpdate) {
  if(m_Identity != value) {
    T last = m_Identity;
    m_Identity = value;
    if(andUpdate)
      ForceUpdate();
    for(auto onIdentityChanged : m_IdentityChanged)
      (*onIdentityChanged)(this, last, m_Identity);
  }
}

template<typename T>
const T& Inspectable<T>::GetValue(bool andForceUpdate) {
  if(andForceUpdate)
    ForceUpdate();
  return m_LastValue;
}

namespace xoins {
  namespace internal {
    template<typename T>
    bool TransformationPredicate(InspectableTransformation<T>* a, InspectableTransformation<T>*b) {
      return a->GetPriority() > b->GetPriority();
    }
  }
}

template<typename T>
void Inspectable<T>::SortTransformations() {
  std::sort(m_Transformations.begin(), m_Transformations.end(), xoins::internal::TransformationPredicate<T>);
}

//////////////////////////////////////////////////////////////////////////////////////////
// InspectableScopedTransformation
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
InspectableScopedTransformation<T>::InspectableScopedTransformation(Inspectable<T>* inspectable, bool updateOnDestroy)
: m_Inspectable(inspectable),
m_UpdateOnDestroy(updateOnDestroy)
{
}

template<typename T>
InspectableScopedTransformation<T>::InspectableScopedTransformation(Inspectable<T>* inspectable,
                                                                    TTransformFunc func,
                                                                    int priority,
                                                                    bool enabled,
                                                                    bool andUpdate,
                                                                    bool updateOnDestroy)
: m_Inspectable(inspectable),
m_Transformation(func, priority, enabled),
m_UpdateOnDestroy(updateOnDestroy)
{
  if(m_Inspectable) {
    m_Inspectable->AddTransformation(&m_Transformation);
    if(andUpdate)
      m_Inspectable->ForceUpdate();
  }
}

template<typename T>
InspectableScopedTransformation<T>::~InspectableScopedTransformation() {
  if(m_Inspectable) {
    m_Inspectable->RemoveTransformation(&m_Transformation);
    if(m_UpdateOnDestroy)
      m_Inspectable->ForceUpdate();
  }
}

template<typename T>
void InspectableScopedTransformation<T>::Set(TTransformFunc func,
                                             int priority,
                                             bool enabled,
                                             bool andUpdate) {
  m_Transformation.Set(func, priority, enabled);
  if(m_Inspectable && andUpdate)
    m_Inspectable->ForceUpdate();
}

template<typename T>
void InspectableScopedTransformation<T>::SetUpdateOnDestroy(bool updateOnDestroy) {
  m_UpdateOnDestroy = updateOnDestroy;
}

template<typename T>
void InspectableScopedTransformation<T>::Enable(bool andUpdate) {
  m_Transformation.Enable();
  if(m_Inspectable && andUpdate) {
    m_Inspectable->ForceUpdate();
  }
}

template<typename T>
void InspectableScopedTransformation<T>::Disable(bool andUpdate) {
  m_Transformation.Disable();
  if(m_Inspectable && andUpdate) {
    m_Inspectable->ForceUpdate();
  }
}

template<typename T>
bool InspectableScopedTransformation<T>::IsEnabled() const {
  return m_Transformation.IsEnabled();
}

template<typename T>
int InspectableScopedTransformation<T>::GetPriority() const {
  return m_Transformation.GetPriority();
}

template<typename T>
const std::function<void(T&)>& InspectableScopedTransformation<T>::GetTransformFunc() const {
  return m_Transformation.GetTransformFunc();
}

template<typename T>
void InspectableScopedTransformation<T>::operator()(T& input) {
  m_Transformation(input);
}

//////////////////////////////////////////////////////////////////////////////////////////
// InspectableScopedValueChangedFunc
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
InspectableScopedValueChangedFunc<T>::InspectableScopedValueChangedFunc()
: m_Inspectable(nullptr),
m_OnValueChanged()
{
}

template<typename T>
InspectableScopedValueChangedFunc<T>::InspectableScopedValueChangedFunc(Inspectable<T>* inspectable, TValueChangedFunc func)
: m_Inspectable(inspectable),
m_OnValueChanged(func)
{
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->AddOnValueChanged(&m_OnValueChanged);
}

template<typename T>
InspectableScopedValueChangedFunc<T>::~InspectableScopedValueChangedFunc() {
  if(m_Inspectable && m_OnValueChanged) // todo: ensure all other func assignments check for null.
    m_Inspectable->RemoveOnValueChanged(&m_OnValueChanged);
}

template<typename T>
void InspectableScopedValueChangedFunc<T>::Set(Inspectable<T>* inspectable,
                                               TValueChangedFunc func) {
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->RemoveOnValueChanged(m_OnValueChanged);
  m_Inspectable = inspectable;
  m_OnValueChanged = func;
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->AddOnValueChanged(func);
}

template<typename T>
void InspectableScopedValueChangedFunc<T>::SetInspectable(Inspectable<T>* inspectable) {
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->RemoveOnValueChanged(m_OnValueChanged);
  m_Inspectable = inspectable;
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->AddOnValueChanged(m_OnValueChanged);
}

template<typename T>
void InspectableScopedValueChangedFunc<T>::SetFunc(TValueChangedFunc func) {
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->RemoveOnValueChanged(m_OnValueChanged);
  m_OnValueChanged = func;
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->AddOnValueChanged(m_OnValueChanged);
}

//////////////////////////////////////////////////////////////////////////////////////////
// InspectableScopedIdentityChangedFunc
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
InspectableScopedIdentityChangedFunc<T>::InspectableScopedIdentityChangedFunc()
: m_Inspectable(nullptr),
m_OnValueChanged()
{
}

template<typename T>
InspectableScopedIdentityChangedFunc<T>::InspectableScopedIdentityChangedFunc(Inspectable<T>* inspectable,
                                                                              TValueChangedFunc func)
: m_Inspectable(inspectable),
m_OnValueChanged(func)
{
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->AddOnIdentityChanged(&m_OnValueChanged);
}

template<typename T>
InspectableScopedIdentityChangedFunc<T>::~InspectableScopedIdentityChangedFunc() {
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->RemoveOnIdentityChanged(&m_OnValueChanged);
}

template<typename T>
void InspectableScopedIdentityChangedFunc<T>::Set(Inspectable<T>* inspectable,
                                                  TValueChangedFunc func) {
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->RemoveOnIdentityChanged(m_OnValueChanged);
  m_Inspectable = inspectable;
  m_OnValueChanged = func;
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->AddOnIdentityChanged(func);
}

template<typename T>
void InspectableScopedIdentityChangedFunc<T>::SetInspectable(Inspectable<T>* inspectable) {
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->RemoveOnIdentityChanged(m_OnValueChanged);
  m_Inspectable = inspectable;
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->AddOnIdentityChanged(m_OnValueChanged);
}

template<typename T>
void InspectableScopedIdentityChangedFunc<T>::SetFunc(TValueChangedFunc func) {
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->RemoveOnIdentityChanged(m_OnValueChanged);
  m_OnValueChanged = func;
  if(m_Inspectable && m_OnValueChanged)
    m_Inspectable->AddOnIdentityChanged(m_OnValueChanged);
}

#define FormInspectableTypedef(xoinsType) \
  typedef xoinsType<bool>                 xoinsType##B;\
  typedef xoinsType<float>                xoinsType##F;\
  typedef xoinsType<double>               xoinsType##D;\
  typedef xoinsType<int>                  xoinsType##I; \
  typedef xoinsType<unsigned>             xoinsType##U; \
  typedef xoinsType<unsigned long long>   xoinsType##ULL; \
  typedef xoinsType<long long>            xoinsType##LL;

FormInspectableTypedef(Inspectable);
FormInspectableTypedef(InspectableTransformation);
FormInspectableTypedef(InspectableScopedTransformation);
FormInspectableTypedef(InspectableScopedValueChangedFunc);
FormInspectableTypedef(InspectableScopedIdentityChangedFunc);

#undef FormInspectableTypedef

#ifdef xoins_list_internal
#undef xoins_list
#endif

#ifdef xoins_list_add_internal
#undef xoins_list_add
#endif

#ifdef xoins_list_erase_internal
#undef xoins_list_erase
#endif
