# AnalogSelector Library for Arduino

This library provides an easy and robust way to read a potentiometer or other analog input as a multi-selector switch.

## Getting Started

```cpp
AnalogSelector selector(Pin, NumPositions);

void setup() {
	selector.begin();
}

void loop() {
	int option = selector.getSelection();
}
```

## License

This library is licensed under the terms of the [MIT license](https://opensource.org/licenses/MIT). See the [LICENSE](LICENSE) file for more information.
