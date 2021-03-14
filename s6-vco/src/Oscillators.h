/*
  ==============================================================================

	VCF.h
	Author:  Jan Janssen & Jarno Verheesen

  ==============================================================================
*/

#pragma once

#include <cstdlib>
#include <cstdint>
#include <cmath>

#define PI 3.14159265358979323846

namespace VeJa
{
	namespace Plugins
	{
		namespace Oscillators
		{
			template <class T > class Oscillator
			{
			public:

				Oscillator(uint32_t samplerate
					, uint32_t defaultFrequency
					, size_t tableSize) : _samplerate(samplerate)
					, _wavetableSize(tableSize)
					, _sineTable(new T[tableSize])
					, _sawTable(new T[tableSize])
					, _squareTable(new T[tableSize])
					, _triangleTable(new T[tableSize])
					, _noiseTable(new T[tableSize])
					, _frequency(defaultFrequency)
					, _phase(static_cast<T>(0))
					, _increment(static_cast<T>(0))
					, _pulseWidth(static_cast<T>(0.5))
					, _hold(0.f)
					,_holdCounter(0)
					, _delayCounter(0)
				{
					GenerateWavetable();
				}

				~Oscillator() {}

				void NewFrequency(T frequency)
				{
					_frequency = frequency;
					_period = static_cast<T>((1 / _frequency) / (1.f / _samplerate));
				}

				void SetPulseWidth(float width)
				{
					_pulseWidth = width;
				}

				void SetDelay(float delayInSeconds)
				{
					if (delayInSeconds == 0)
					{
						_delayCounter = 0;
					}
					else
					{
						_delayCounter = static_cast<uint32_t>(_samplerate * delayInSeconds);
					}
				}

				void UpdateFrequency()
				{
					_increment = _frequency * _wavetableSize / _samplerate;
					_phase = static_cast<T>(fmod((_phase + _increment), _wavetableSize));

					_counter++;
					_holdCounter++;

					//update pwm
					if (_counter < (_period * _pulseWidth))
					{
						_square = static_cast<T>(1);
					}
					else
					{
						_square = static_cast<T>(-1);
					}

					//reset
					if (_counter >= _period)
					{
						_counter = 0;
					}

					//delay counter
					if (_delayCounter != 0)
					{
						_delayCounter--;
						return;
					}
				}

				//TODO add interpolation
				T GetDataValue(int form)
				{
					T output = static_cast<T>(0);

					if (_delayCounter != 0)
					{
						return output;
					}

					switch (form)
					{
					case 0:
						output = _sineTable[static_cast<size_t>(_phase)];
						return output;
						break;
					case 1:
						output = _triangleTable[static_cast<size_t>(_phase)];
						return output;
						break;
					case 2:
						output = _sawTable[static_cast<size_t>(_phase)];
						return output;
						break;
					case 3:
						output = _square;
						return output;
						break;
					case 4:
						output = _noiseTable[static_cast<size_t>(_phase)];
						return output;
						break;
					case 5:
						output = SampleAndHold(_phase);
						return output;
						break;
					case 6:
						output = _squareTable[static_cast<size_t>(_phase)];
						return output;
						break;
					default:
						return _sineTable[static_cast<size_t>(_phase)];
						break;
					}
				}

			private:

				void GenerateWavetable()
				{
					// generate wavetables
					for (size_t i = 0; i < _wavetableSize; i++)
					{
						// generate sine wave
						_sineTable[i] = static_cast<T>(sin(2.0 * PI * i / _wavetableSize));
						// generate saw table
						_sawTable[i] = static_cast<T>(((2.0 * i) / _wavetableSize) - 1.0);
						//generate triangle table
						_triangleTable[i] = static_cast<T>((i < _wavetableSize / 2.0) ? (((4.0 * i) / _wavetableSize)) : (3.0 - (4.0 * i / _wavetableSize)));
						//generate noise table
						_noiseTable[i] = GenerateRandomSample();
						//generate square table
						_squareTable[i] = static_cast<T>(((i < _wavetableSize / 2.0) ? 1.0 : -1.0));
					}
				}

				T GenerateRandomSample()
				{
					//Generates additive white Gaussian Noise samples with zero mean and a standard deviation of 1.

					double temp1;
					double temp2;
					double result;
					int p;

					p = 1;

					//nice and dirty
					while (p > 0)
					{
						//rand() function generates an integer between 0 and  RAND_MAX, which is defined in stdlib.h.
						temp2 = (rand() / ((double)RAND_MAX));

						if (temp2 == 0)
							p = 1;
						else
							p = -1;
					}

					temp1 = cos((2.0 * PI) * rand() / ((double)RAND_MAX));
					result = sqrt(-2.0 * log(temp2)) * temp1;

					// return the generated random sample to the caller
					return static_cast<T>(result);
				}

				T SampleAndHold(float phase)
				{
					if (_holdCounter < _period)
					{
						return _hold;
					}
					else
					{
						auto randomIndex = 0 + (rand() % static_cast<uint32_t>(_wavetableSize - 0 + 1));
						_hold = _noiseTable[static_cast<size_t>(randomIndex)];
						_holdCounter = 0;
						return _hold;
					}
				}

				// variables
				uint32_t _samplerate;
				size_t _wavetableSize;

				T* _sineTable;
				T* _sawTable;
				T* _squareTable;
				T* _triangleTable;
				T* _noiseTable;
				T _frequency;
				T _phase;
				T _increment;

				T _square;
				float _pulseWidth;
				uint8_t _oscilatorDelay;
				uint32_t _counter;
				T _period;

				T _hold;
				uint32_t _holdCounter;
				uint32_t _delayCounter;
			};
		}
	}
}