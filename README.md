# xo-inspectable
A public domain single header file module for inspecting and modifying data functionally. C++11 or newer required.

# Example: set & get an inspectable value

Plain and simple, create an inspectable value set to 10, and retrieve it. Set it to 11 (and foce an update of the cached value) and get that updated value.
``` cpp
  Inspectable<float> m_PlayerSpeed(10.0f);
  // Output: Player speed is 10
  std::cout << "Player speed is " << m_PlayerSpeed.GetValue() << std::endl;
 
  m_PlayerSpeed.SetIdentity(11.0f, true); // second param forces an update
    // Output: Player speed is 11
    std::cout << "Player speed is " << m_PlayerSpeed.GetValue() << std::endl;
```

## Example: force update & get an inspectable value

This example isn't too exciting, it sets a value to 10 and gets it twice. Presume some interesting code has executed prior to the `cout` calls and we aren't sure that the cached value returned by `GetValue` is still unchanged.


``` cpp
Inspectable<float> m_PlayerSpeed(10.0f);
  // ... some time later, we aren't sure if m_PlayerSpeed has had some transformations added to it.
  // use the `andUpdate` parameter to GetValue to force it to update, so we can be sure it's valid.
  std::cout << "Player speed is " << m_PlayerSpeed.GetValue(true) << std::endl;
  // ... OR: we can manually update before getting the value.
  m_PlayerSpeed.ForceUpdate();
  std::cout << "Player speed is " << m_PlayerSpeed.GetValue() << std::endl;
```

## Example: scoped transformations with priority.

In this example we create a `m_PlayerSpeed` inspectable with a base value of 10. We watch this value for changes using `watchSpeedChanged`. So long as it is in scope we will call the attached `OnSpeedChanged` function on every change.

`Inspectable` types do not update every time a transformation is added by default, but for the simplicity of this example we've set the `andUpdate` boolean parameter to true on every `InspectableScopedTransformation` constructor call. This forces the `Inspectable` object to update its resulting value and notify watchers (like `watchSpeedChanged`) of any value changes if there are one.

This example adds a few transformations to the `m_PlayerSpeed` value, and you will notice that they are removed automatically when the `InspectableScopedTransformation` goes out of scope. This can be seen on the final output after `m_MudTrap` leaves scope.

``` cpp
  Inspectable<float> m_PlayerSpeed(10.0f);
  InspectableScopedValueChangedFunc<float> watchSpeedChanged(&m_PlayerSpeed, OnSpeedChanged);

  InspectableScopedTransformation<float> m_PowerPill(&m_PlayerSpeed, [](float& val) {
    val *= 2.0f;
  }, 0, true, true); // output: speed value was 10 and is now 20

  InspectableScopedTransformation<float> m_EpicKillStreek(&m_PlayerSpeed, [](float& val) {
    val += 2.0f;
  }, 0, true, true); // output: speed value was 20 and is now 22

  {
    // Note: the priority value is 1, so it will execute before the other transformations rather than the order it was added.
    InspectableScopedTransformation<float> m_MudTrap(&m_PlayerSpeed, [](float& val) {
      val -= 5.0f;
    }, 1, true, true, true); // output: speed value was 22 and is now 12
    // when m_MudTrap goes out of scope, the transformation is destroyed. The last param 'true' will force an update on destroy.
  } // output: speed value was 12 and is now 22
```
Just a helper function for printing the changes.
``` cpp
void OnSpeedChanged(Inspectable<float>* inspectable, const float& previousSpeed, const float& newSpeed) {
  std::cout << "speed value was " << previousSpeed << " and is now " << newSpeed << std::endl;
}
```

# Todo 1.0:
- I would like to refactor to include an optional `xo` namespace
- Refactor the boolean parameters to use a single bitflag. Most bool parameters are common throughought the file, and readability is poor having three bools in a row. What the hell does `true, false, true` indicate versus `true, true, false`. Not very readable!
- Along with the bitflags we could improve `andUpdate` to have a variation where it updates only if there's a dirty flag set which would happen any time transforms are added/removed/enabled/disabled. The use case is not being sure if transforms have been added, while still being certain that all transforms are purely functional.
- Improve naming conventions so you don't have huge names for common types like `InspectableScopedValueChanged<type>`. 
- locally unnamed namespace helper function(s)
- Spend some time trying out the customization features with overriding list/array. Consider usage with some other list types by third parties.
- Test on various compilers.
- Publish test cases.
- Write more examples including more supported variations and at least one inspectable of a custom type.

# Versions:
### 0.1 (july 2016): Initial commit.

# Street Cred
An inspired implementation of [Twisted Oaks Lens](https://github.com/twistedoakstudios/tounityutils/tree/master/assets/lenses)
 
Also inspired by [Sean Barrett's stb](https://github.com/nothings).

# LICENSE
This software is dual-licensed to the public domain and under the following license: 

>You are granted a perpetual, irrevocable license to copy, modify, publish, and distribute this file as you see fit.
