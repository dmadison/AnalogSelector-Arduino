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

#ifndef ANALOG_SELECTOR_H
#define ANALOG_SELECTOR_H

#include <stdint.h>


/**
 * @brief Filter class for converting a position to a selector
 * 
 * In place of simple division / rescaling, this class implements deadzones
 * between each selection segment. These deadzones provide hysteresis in
 * order to guard against noise.
 * 
 * For example, using simple division it is possible for an input to sit on
 * the fence between two positions. As noise influences the reading, it will
 * move slightly below (selector 1) or slightly above (selector 2) the
 * threshold between these two options without further user input.
 * 
 * The deadzones implemented in this class prevent this behavior. Instead of
 * a single threshold between the two positions there is a buffer zone. While
 * input is in this deadzone the selection does not change. When it has crossed
 * over from the deadzone threshold into the area for the next position, the
 * thresholds are updated so that the input value must cross *back* over the
 * deadzone to switch to the previous value.
 * 
 * For example, with 2 selection options in a range from 0-100 with a 10%
 * deadzone in the center, you would have the following input zones:
 * 
 * ```
 * Selector 1:  0 - 44
 * Deadzone:   45 - 55
 * Selector 2: 56 - 100
 * ```
 * 
 * If the input is within the selector 1 range, it has to travel above 55 in order to
 * activate selector 2. Once the selection has swapped to 2 however, it must travel
 * below 45 to get back to selector 1. This allows the selector to sit (noisily, if
 * it must) between the two selector zones without changing the program output.
*/
class AnalogSelectorFilter {
public:
	/**
	 * Class constructor
	 * 
	 * @param rMin   Minimum input range
	 * @param rMax   Maximum input range
	 * @param numPos Number of selector positions for the output
	 * @param dz     Percentage of the range to act as a deadzone (0 - 1.0)
	*/
	AnalogSelectorFilter(int rMin, int rMax, unsigned int numPos, float dz);

	/**
	 * Runs the filter to obtain the current position of the selector
	 * 
	 * @param pos Input position
	 * @returns   The current position, indexed from 0
	*/
	unsigned int getPosition(int pos);

	/**
	 * Sets the input range for the filter
	 * 
	 * Any input values for AnalogSelectorFilter::getPosition(int) outside of
	 * this range will be clipped to these values
	 * 
	 * @param rMin Minimum input range
	 * @param rMax Maximum input range
	*/
	void setRange(int rMin, int rMax);

	/**
	 * Sets the number of output positions for the filter
	 * 
	 * @param numPos Number of output positions to set
	*/
	void setNumPositions(unsigned int numPos);

	/**
	 * Sets the size of the filter deadzones
	 * 
	 * Larger deadzones are more resilient to noise but will require the user
	 * to push the input farther to change between positions.
	 * 
	 * @param dz Deadzone size as a percentage, 0 - 1.0
	*/
	void setDeadzone(float dz);

private:
	enum Direction { Upper, Lower };  ///< Simple enum to handle direction selection

	/**
	 * Calculates the boundary for changing positions
	 * 
	 * This utility function will calculate the bounds of the current selection,
	 * taking into account the current deadzones. If the input moves past these
	 * bounds the selection has changed.
	 * 
	 * @param i   Current selection index, indexed from 0
	 * @param dir Which boundary to calculate, Upper or Lower
	 * @returns   The calculated boundary, in the user range
	*/
	int calculateEdge(unsigned int i, Direction dir) const;

	/**
	 * Recalculates the width of each selector and deadzone area
	 * 
	 * For efficiency reasons the class buffers the width of each selector area
	 * and deadzone area. These are required for calculating the boundary
	 * thresholds for the current selection.
	 * 
	 * If the range, number of positions, or deadzone size changes, these widths
	 * need to be recalculated. A flag (AnalogSelectorFilter::configChanged) is
	 * set whenever those values are updated, and this function calculates the
	 * resulting widths and clears it.
	*/
	void recalculateWidths();

	/**
	 * Calculates the position of the selector from the input
	 * 
	 * @param pos      Input position, in the user range
	 * @param relative Whether to use relative calculations. If 'true', selection
	 *                 calculation begins from the current selection and moves up
	 *                 or down. If 'false', selection calculation begins from the
	 *                 bottom and moves to the top. Relative calculations are more
	 *                 efficient, but they require a known starting position.
	 * @returns        The position of the selector, indexed from 0
	*/
	unsigned int calculateSelection(int pos, bool relative);

	// Config data
	bool configChanged;             ///< flag that's set if the config is changed, so we can recalculate widths
	int rangeMin;                   ///< the lower bound of the input range
	int rangeMax;                   ///< the upper bound of the input range
	unsigned int numPositions;      ///< the number of output positions for the selector
	float deadzoneSize;             ///< the size of the deadzone segments, 0 - 1.0 as a percentage of the total range

	// Calculated Config Widths
	unsigned int selectorWidth;     ///< the width of each selector area, in user units
	unsigned int deadzoneWidth;     ///< the width of each deadzone area, in user units

	// Current Status data
	int edgeLow;                    ///< the lower edge of the current selection bound, in user units
	unsigned int currentSelection;  ///< the current selection, buffered for efficiency
	int edgeHigh;                   ///< the upper edge of the current selection bound, in user units
};


/**
 * @brief Analog selector class for Arduino analog inputs
 */
class AnalogSelector {
public:
	/**
	 * Class constructor
	 * 
	 * @param pin    Analog pin to read from
	 * @param numPos Number of positions in the selector
	 * @param rMin   Minimum input range. Defaults to 0
	 * @param rMax   Maximum input range. Defaults to 1023, the max output of `analogRead()`
	*/
	AnalogSelector(unsigned int pin, unsigned int numPos, int rMin = 0, int rMax = 1023);

	/**
	 * Initializes the pin by setting it to 'input'
	*/
	void begin();

	/**
	 * Runs the filter to obtain the current position of the selector
	 *
	 * @returns The current position, indexed from 0
	*/
	unsigned int getPosition();

	/** @copydoc AnalogSelectorFilter::setRange(int, int) */
	void setRange(int rMin, int rMax);

	/** @copydoc AnalogSelectorFilter::setNumPositions(unsigned int) */
	void setNumPositions(unsigned int numPos);

	/** @copydoc AnalogSelectorFilter::setDeadzone(float) */
	void setDeadzone(float dz);

private:
	AnalogSelectorFilter filter;  ///< AnalogSelectorFilter instance, via composition for a cleaner interface
	const unsigned int Pin;       ///< The analog pin, in Arduino numbering, used by this class
};

#endif
