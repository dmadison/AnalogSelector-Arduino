/*
 *  Project     AnalogSelector Library
 *  @author     David Madison
 *  @link       github.com/dmadison/AnalogSelector-Arduino
 *  @license    MIT - Copyright (c) 2023 David Madison
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *  Example:      EngineOrderTelegraph
 *  Description:  Communicate the speed of your ship to the engine room using a
 *                simple rotary potentiometer.
 *  Refrence:     https://en.wikipedia.org/wiki/Engine_order_telegraph
 */

#include <AnalogSelector.h>


const char* DialPositions[] = {
	"Emergency Astern",
	"Full Astern",
	"Half Astern",
	"Slow Astern",
	"Stop",
	"Slow Ahead",
	"Half Ahead",
	"Full Ahead",
	"Flank Ahead",
};
const int NumPositions = sizeof(DialPositions) / sizeof(DialPositions[0]);

const int Pin = A0;
AnalogSelector mode(Pin, NumPositions);

int previousMode = -1;


void setup() {
	Serial.begin(115200);
	while (!Serial);

	mode.begin();
	delay(500);
}

void loop() {
	int newMode = mode.getPosition();

	if (newMode != previousMode) {
		Serial.print("Engine mode set to \"");
		Serial.print(DialPositions[newMode]);
		Serial.println("\"");
		previousMode = newMode;
	}
}
