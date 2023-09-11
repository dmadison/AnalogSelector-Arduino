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
 */

#include "AnalogSelector.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif


AnalogSelectorFilter::AnalogSelectorFilter(int rMin, int rMax, unsigned int numPos, float dz)
{
	setRange(rMin, rMax);
	setNumPositions(numPos);
	setDeadzone(dz);

	this->getPosition(rMin);  // initial selection is bottom of the range
}

unsigned int AnalogSelectorFilter::getPosition(int pos) {
	const bool relative = !this->configChanged;
	if (this->configChanged) recalculateWidths();

	return calculateSelection(pos, relative);
}

void AnalogSelectorFilter::setRange(int rMin, int rMax) {
	// swap these if they're reversed
	if (rMax < rMin) {
		const int temp = rMin;
		rMin = rMax;
		rMax = temp;
	}

	this->rangeMin = rMin;
	this->rangeMax = rMax;

	this->configChanged = true;
}

void AnalogSelectorFilter::setNumPositions(unsigned int numPos) {
	if (numPos == 0) numPos = 1;  // can't have 0 segments
	this->numPositions = numPos;
	this->configChanged = true;
}

void AnalogSelectorFilter::setDeadzone(float dz) {
	//if (dz == NAN)     dz = 0.0;
	     if (dz < 0.0) dz = 0.0;
	else if (dz > 1.0) dz = 1.0;

	this->deadzoneSize = dz;
	this->configChanged = true;
}

int AnalogSelectorFilter::calculateEdge(unsigned int i, Direction dir) const {
	if (i < 0) i = 0;

	int edge;

	if (dir == Direction::Upper) {
		edge = this->rangeMin + (this->selectorWidth * (i + 1)) + (this->deadzoneWidth * (i != this->numPositions ? i+1 : i));
	}

	else if (dir == Direction::Lower) {
		edge = this->rangeMin + (this->selectorWidth * i) + (this->deadzoneWidth * (i != 0 ? i - 1 : i));
	}

	else {
		edge = this->rangeMin;  // should never occur, but guarding against '-Wmaybe-uninitialized'
	}

	if (edge < this->rangeMin) edge = this->rangeMin;
	if (edge > this->rangeMax) edge = this->rangeMax;

	return edge;
}

void AnalogSelectorFilter::recalculateWidths() {
	// the total available range in the user scale
	const unsigned int TotalRange = abs(rangeMax - rangeMin);

	// Deadzone calculations first
	// --------------------------------

	// saving (1 * numPositions) for a minimum active area, so we don't
	// have 100% deadzone at the limits
	const unsigned int DeadzoneRange = TotalRange - numPositions;

	// accounting for deadzones between every position with none at the ends
	const unsigned int NumDeadzones = (this->numPositions) - 1;

	// the absolute limit for a deadzone, assuming a deadzone size of 1.0
	const unsigned int MaxDeadzoneWidth = (NumDeadzones != 0) ? (DeadzoneRange / NumDeadzones) : 0;

	// the width of each deadzone segment, in the units of the range
	this->deadzoneWidth = (float)MaxDeadzoneWidth * this->deadzoneSize;

	// Selection calculations second
	// --------------------------------

	// the total selector range is the area that is left after the deadzone cals
	const unsigned long SelectorRange = TotalRange - (this->deadzoneWidth * NumDeadzones);

	// the width of each selector segment is the selector range divided by the number of positions
	this->selectorWidth = SelectorRange / this->numPositions;

	// Clear the config flag and continue
	// --------------------------------
	this->configChanged = false;

#if 0
	Print& output = Serial;

	output.println("Recalculating...");

	output.print("Range Min: ");
	output.println(this->rangeMin);
	output.print("Range Max: ");
	output.println(this->rangeMax);
	output.print("Total Range: ");
	output.println(TotalRange);

	output.print("Deadzone Width: ");
	output.println(this->deadzoneWidth);
	output.print("Selector Range: ");
	output.println(this->selectorWidth);

	output.println("Edges:");
	for (unsigned int i = 0; i < this->numPositions; i++) {
		output.print("S"); output.print(i); output.print(" ");
		output.print(this->calculateEdge(i, Direction::Lower));
		output.print(" - ");
		output.print(this->calculateEdge(i, Direction::Upper));
		output.println();
	}
#endif

}

unsigned int AnalogSelectorFilter::calculateSelection(int pos, bool relative) {
	     if (pos < rangeMin) pos = rangeMin;
	else if (pos > rangeMax) pos = rangeMax;

	// if we're not using relative positioning, calculate from the bottom
	// to the top starting from 0
	//
	// if we are using relative positioning, calculate going up if we're above the
	// upper bound
	if (!relative || pos > edgeHigh) {
		int start = !relative ? 0 : this->currentSelection;

		for (int i = start; i < (int) numPositions; i++) {
			const int UpperEdge = calculateEdge(i, Direction::Upper);
			if (pos > UpperEdge) continue;  // if we're above the upper edge we can't be in this selection
		
			// otherwise, we've found our selection! assign and quit
			this->currentSelection = i;
			this->edgeLow = calculateEdge(i, Direction::Lower);
			this->edgeHigh = UpperEdge;
			break;
		}
	}

	// if below the lower limit, start calculating going downwards
	else if (pos < edgeLow) {
		for (int i = currentSelection; i >= 0; i--) {
			const int LowerEdge = calculateEdge(i, Direction::Lower);
			if (pos < LowerEdge) continue;

			this->currentSelection = i;
			this->edgeLow = LowerEdge;
			this->edgeHigh = calculateEdge(i, Direction::Upper);
			break;
		}
	}

	// if we're inside the bounds we haven't changed
	else {}
	
#if 0
	Print& output = Serial;

	output.print("Current Selection: ");
	output.println(this->currentSelection);

	output.print("New Lower Edge: ");
	output.println(this->edgeLow);

	output.print("New Upper Edge: ");
	output.println(this->edgeHigh);
#endif

	return this->currentSelection;
}


AnalogSelector::AnalogSelector(unsigned int pin, unsigned int numPos, int rMin, int rMax)
	: filter(rMin, rMax, numPos, 0.2), Pin(pin)
{}

void AnalogSelector::begin() {
#ifdef ARDUINO
	pinMode(this->Pin, INPUT);
#endif
	this->getPosition();  // set initial position
}

unsigned int AnalogSelector::getPosition() {
#ifdef ARDUINO
	const int reading = analogRead(this->Pin);
	return this->filter.getPosition(reading);
#else
	return 0;  // no Arduino support, can't read
#endif
}

void AnalogSelector::setRange(int rMin, int rMax) {
	this->filter.setRange(rMin, rMax);
}

void AnalogSelector::setNumPositions(unsigned int numPos) {
	this->filter.setNumPositions(numPos);
}

void AnalogSelector::setDeadzone(float dz) {
	this->filter.setDeadzone(dz);
}
