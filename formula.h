#ifndef FORMULA
#define FORMULA
#include <string>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <memory>
#include <type_traits>
#include <functional>
#include "utils.h"

#define FORMULA_FLOATING_TYPE float

enum formulaType : unsigned char {
	CONSTANT, // this one should be destroyed during creation
	CALL, // returns one of the variables given in the request
	SUM, // two operands
	SUM_CONST, // one of the summed operands is a constant, ALWAYS WRITTEN SECOND
	MULTIPLICATION, // two operands
	MULTIPLICATION_CONST, // one of the multiplied operands is a constant
	POWER, // power of two operands
	SQUARE, // second power of an operand
	CUBE, // third power of an operand
	SQUARE_ROOT, // square root of an operand
	INVERSE, // inverse of an operand
	POWER_CONSTANT, // power by a constant
	UNNATURAL_EXPONENTIAL, // power of a constant
	EXPONENTIAL, // unary
	SINE,
	ARCSINE,
	COSINE,
	ARCCOSINE,
	TANGENT,
	ARCTANGENT,
	LOGARITHM, // two operands
	NATURAL_LOGARITHM, // single operand
	ABSOLUTE_VALUE, // Cannot be called just ABSOLUTE, mingw makes some absurd error reports
	FUNC_ENTRY, // Function that uses numbers given by the user
	FUNC_UNARY, // Function that uses computed values
	FUNC_BINARY // Function that uses computed values
};

template <typename scalar = FORMULA_FLOATING_TYPE>
struct formula {
	formula() : data(0) {}
	formula(const formula& copied) {
		valueMaker = copied.valueMaker;
		data = copied.data;
		std::cerr << "Copy relocation(1)\n";
		for (unsigned int i = 0; i < data.size(); i++) {
			data[i].relocate((long int)&data[i] - (long int)&copied.data[i]);
			std::cerr << "Moved variable from " << (long int)&copied.data[i] << " to " << (long int)&data[i] << std::endl;
		}
	}
	formula(formula&& moved) {
		valueMaker = moved.valueMaker;
		data.swap(moved.data);
	}
	formula(FORMULA_FLOATING_TYPE constant) : data(1) { data[0] = subFormula(constant); }
	scalar operator () (const scalar* const variables) const {
		return data[0].compute(variables, valueMaker);
	}
	void operator = (const formula& assigned) {
		valueMaker = assigned.valueMaker;
		data = assigned.data;
		for (unsigned int i = 0; i < data.size(); i++) {
			data[i].relocate((long int)&data[i] - (long int)&assigned.data[i]);
		}
	}
	operator bool() const { return !data.empty(); }
	void clear() { data.clear(); }
    std::string print(const std::vector<std::string>& vars,
            const std::vector<std::pair<std::string, scalar (*)(scalar)>>& unaryFuncs =
            std::vector<std::pair<std::string, scalar (*)(scalar)>>()) {
        return data[0].print(vars, unaryFuncs);
	}
	void setValueMaker(std::function<scalar(FORMULA_FLOATING_TYPE)> valueMakerSet) const {
		valueMaker = valueMakerSet;
		// Is mutable, each computation may need to create an array of different size
	}

	static inline formula<scalar> constant(const FORMULA_FLOATING_TYPE& term) {
		std::vector<subFormula> made(1);
		made[0] = subFormula(term);
		return formula<scalar>(made);
	}
	static inline formula<scalar> sum(const formula& term1, const formula& term2) {
		return genericBinary(SUM, SUM_CONST, term1, term2);
	}
	static inline formula<scalar> subtraction(const formula& term1, const formula& term2) {
		if (term2.isConstant()) {
			formula term2b(term2);
			FORMULA_FLOATING_TYPE& flipping = term2b.getConstant();
			flipping *= -1;
			return sum(term1, term2b);
		}
		return sum(term1, multiplication(-1, term2));
	}
	static inline formula<scalar> multiplication(const formula& term1, const formula& term2) {
		return genericBinary(MULTIPLICATION, MULTIPLICATION_CONST, term1, term2);
	}
	static inline formula<scalar> division(const formula& term1, const formula& term2) {
		if (term2.isConstant()) {
			formula term2b(term2);
			FORMULA_FLOATING_TYPE& flipping = term2b.getConstant();
			flipping = 1 / flipping;
			return multiplication(term1, term2b);
		}
		return multiplication(term1, inverse(term2));
	}
	static inline formula<scalar> power(const formula& term1, const formula& term2) {
		if (term2.isConstant()) {
			if (term1.isConstant()) {
				return formula<scalar>(pow(term1.getConstant(), term2.getConstant()));
			}
			formulaType fits = POWER_CONSTANT;
			const FORMULA_FLOATING_TYPE& constant = term2.getConstant();
			if (constant == -1) fits = INVERSE;
			else if (constant == 0) return formula(1); // Careful, a wild return here
			else if (constant == 0.5) fits = SQUARE_ROOT;
			else if (constant == 1) return formula(term1); // Another wild return here
			else if (constant == 2) fits = SQUARE;
			else if (constant == 3) fits = CUBE;
			if (fits != POWER_CONSTANT) {
				std::vector<subFormula> made(1 + term1.data.size());
				made[0] = subFormula(fits, &made[1]);
				for (unsigned int i = 0; i < term1.data.size(); i++) {
					made[i + 1] = term1.data[i];
					made[i + 1].relocate((long int)&made[i + 1] - (long int)&term1.data[i]);
				}
				return formula<scalar>(made);
			} // else go through the usual way
		}
		return genericBinary(POWER, UNNATURAL_EXPONENTIAL, term1, term2);
	}
	static inline formula<scalar> square(const formula& term) {
		return genericUnary(SQUARE, term);
	}
	static inline formula<scalar> cube(const formula& term) {
		return genericUnary(CUBE, term);
	}
	static inline formula<scalar> squareRoot(const formula& term) {
		return genericUnary(SQUARE_ROOT, term);
	}
	static inline formula<scalar> inverse(const formula& term) {
		return genericUnary(INVERSE, term);
	}
	static inline formula<scalar> exponential(const formula& term) {
		return genericUnary(EXPONENTIAL, term);
	}
	static inline formula<scalar> sine(const formula& term) {
		return genericUnary(SINE, term);
	}
	static inline formula<scalar> arcsine(const formula& term) {
		return genericUnary(ARCSINE, term);
	}
	static inline formula<scalar> cosine(const formula& term) {
		return genericUnary(COSINE, term);
	}
	static inline formula<scalar> arccosine(const formula& term) {
		return genericUnary(ARCCOSINE, term);
	}
	static inline formula<scalar> tangent(const formula& term) {
		return genericUnary(TANGENT, term);
	}
	static inline formula<scalar> arctangent(const formula& term) {
		return genericUnary(ARCTANGENT, term);
	}
	static inline formula<scalar> logarithm(const formula& term1, const formula& term2) {
		// I suppose this will be used so rarely that the constant optimalisation will not be needed
		return fullBinary(LOGARITHM, term1, term2);
	}
	static inline formula<scalar> naturalLogarithm(const formula& term) {
		return genericUnary(NATURAL_LOGARITHM, term);
	}
	static inline formula<scalar> absolute(const formula& term) {
		return genericUnary(ABSOLUTE_VALUE, term);
	}
	static inline formula<scalar> call(unsigned char index) {
		std::vector<subFormula> made(1);
		made[0] = subFormula(CALL, index);
		return formula<scalar>(made);
	}
	static inline formula<scalar> func(scalar (*func)(const scalar*)) {
		std::vector<subFormula> made(1);
		made[0] = subFormula(FUNC_ENTRY, func);
		return formula<scalar>(made);
	}
	static inline formula<scalar> func(scalar (*func)(scalar), const formula& term) {
		std::vector<subFormula> made(1 + term.data.size());
		made[0] = subFormula(FUNC_UNARY, func, &made[1]);
		for (unsigned int i = 0; i < term.data.size(); i++) {
			made[i + 1] = term.data[i];
			made[i + 1].relocate((long int)&made[i + 1] - (long int)&term.data[i]);
		}
		return formula<scalar>(made);
	}
	static formula<scalar> parseFormula(const char*& string, const std::vector<std::string> &vars,
									const std::vector<std::pair<std::string, scalar (*)(scalar)>>& unaryFuncs
									= std::vector<std::pair<std::string, scalar (*)(scalar)>>()) {
		//std::cerr << "Parsing formula " << string << std::endl;

		formula<scalar> added;
		formula<scalar> multiplied;
		formula<scalar> powered;
		static formula<scalar> minusOne = formula<scalar>::constant(-1);
		bool gonnaDivide = false;
		int minus = 0; // More of them can be chained, we need only the parity
		char lastTerm = 'n';

		// Prepare lambdas for reading numbers
		auto readString = [&] (const char* needed) -> bool {
			for ( ;needed[0] != 0; needed++, string++) {
				if (needed[0] != string[0]) {
					return false;
				}
			}
			return true;
		};

		auto append = [&] (formula<scalar>& appended, char operation) -> void {
			if (gonnaDivide) {
				// This occurs when the last operation was division,
				// is just the same as multiplication by the inverse of the number
				appended = formula<scalar>::inverse(appended);
			}
			if (operation == '^') {
				if (powered) {
					powered = formula<scalar>::power(powered, appended);
				} else powered = appended;
			} else if (operation == '*') {
				if (powered) {
					if (multiplied) {
						multiplied = formula<scalar>::multiplication(multiplied,
											formula<scalar>::power(powered, appended));
						powered.clear();
					} else multiplied = formula<scalar>::power(powered, appended);
				}
				else if (multiplied) {
					multiplied = formula<scalar>::multiplication(multiplied, appended);
				} else multiplied = appended;
			} else if (operation == '+') {
				if (powered) {
					if (multiplied) {
						if (minus % 2)
							multiplied = formula<scalar>::multiplication(multiplied, minusOne);
						minus = 0;
						if (added) {
							added = formula<scalar>::sum(added, formula<scalar>::multiplication(
								multiplied, formula<scalar>::power(powered, appended)));
						} else {
							added = formula<scalar>::multiplication(multiplied,
													formula<scalar>::power(powered, appended));
						}
						multiplied.clear();
					} else {
						if (added) {
							if (minus % 2)
								added = formula<scalar>::sum(added, formula<scalar>::multiplication(
												formula<scalar>::power(powered, appended), minusOne));
							else added = formula<scalar>::sum(added,
														formula<scalar>::power(powered, appended));
							minus = 0;
						} else {
							added = formula<scalar>::power(powered, appended);
						}
					}
					powered.clear();
				} else {
					if (multiplied) {
						if (minus % 2)
							multiplied = formula<scalar>::multiplication(multiplied, minusOne);
						minus = 0;
						if (added) {
							added = formula<scalar>::sum(added,
									formula<scalar>::multiplication( multiplied, appended));
						} else added = formula<scalar>::multiplication( multiplied, appended);
						multiplied.clear();
					} else {
						if (minus % 2)
							appended = formula<scalar>::multiplication(minusOne, appended);
						minus = 0;
						if (added) added = formula<scalar>::sum(added, appended);
						else added = appended;
					}
				}
			}
		};

		formula<scalar> term;
		for ( ; ; string++) {
			// First, read a term (bracketed subformula, variable or number)
			if (string[0] == ' ') {
			} if (string[0] == '$') {
				string++;
				if (string[0] == 0) break;
				std::unique_ptr<std::string> varName(new std::string());
				while ((string[0] >= '0' && string[0] <= '9') || (string[0] >= 'a'
					   && string[0] <= 'z') || (string[0] >= 'A'
					   && string[0] <= 'Z') || string[0] == '_' )
				{
					varName->push_back(string[0]);
					if (string[0] == 0) break; else string++;
				}
				if (string[0] == '|') {
					string++;
				}
				for (unsigned int i = 0; i < vars.size(); i++) {
					if (*varName == vars[i]) {
						term = formula<scalar>::call(i);
						goto found;
					}
				}
				throw (std::runtime_error("Undefined variable " + *varName));
				// It reaches this section only if nothing is found
				found:;
				lastTerm = '$';
			} else if (string[0] == '(') {
				string++;
				term = parseFormula(string, vars, unaryFuncs); // Terms in brackets are done recursively
				string++;
				lastTerm = '(';
			} else if ((string[0] >= '0' && string[0] <= '9') ||
					   (string[0] == '-' && string[1] >= '0' && string[1] <= '9')) {
				short int sign = 1;
				if (string[0] == '-') {
					sign = -1;
					if (string[0] != 0) string++;
				}
				float result = 0;
				while (string[0] >= '0' && string[0] <= '9') {
					result *= 10;
					result += string[0] - '0';
					string++;
				}
				if (string[0] == '.' || string[0] == ',') {
					string++;
					float divisor = 1;
					float decimal = 0;
					while (string[0] >= '0' && string[0] <= '9') {
						decimal *= 10;
						divisor *= 10;
						decimal += string[0] - '0';
						string++;
					}
					result += decimal / divisor;
				}
				term = formula<scalar>::constant(result * sign);
				lastTerm = '1';
			} else if ((string[0] >= 'a' && string[0] <= 'z') || (string[0] >= 'A' && string[0] <= 'Z')) {
				formula<scalar>(*got)(const formula<scalar>& term) = nullptr;
				if (string[0] == 'a') {
					if (string[1] == 's') {
						if (readString("arcsin(")) got = &formula<scalar>::arcsine;
					} else if (string[1] == 'c') {
						if (readString("arccos(")) got = &formula<scalar>::arccosine;
					} else if (string[1] == 't') {
						if (readString("arctan(")) got = &formula<scalar>::arctangent;
					} else if (string[1] == 'b') {
						if (readString("abs(")) got = &formula<scalar>::absolute;
					}
				} else if (string[0] == 's') {
					if (readString("sin(")) got = &formula<scalar>::sine;
				} else if (string[0] == 'c') {
					if (readString("cos(")) got = &formula<scalar>::cosine;
				} else if (string[0] == 't') {
					if (readString("tan(")) got = &formula<scalar>::tangent;
				} else if (string[0] == 'e') {
					if (readString("exp(")) got = &formula<scalar>::exponential;
				} else if (string[0] == 'l') {
					if (readString("ln(")) got = &formula<scalar>::naturalLogarithm;
				}
				if (got) {
					formula<scalar> within = parseFormula(string, vars, unaryFuncs);
					string++;
					lastTerm = 'f';
					term = (*got)(within);
				} else {
					lastTerm = '?';
					for (unsigned int i = 0; i < unaryFuncs.size(); i++) {
						unsigned int j = 0;
						for ( ; unaryFuncs[i].first[j] != 0; j++)
							if (unaryFuncs[i].first[j] != string[j]) break;
						if (string[j] == '(') {
							string += 2;
							formula<scalar> within = parseFormula(string, vars, unaryFuncs);
							lastTerm = 'f';
							term = formula<scalar>::func(unaryFuncs[i].second, within);
						}
					}
					if (lastTerm == '?') throw (std::runtime_error("Unknown function, use only sin, "
						   "cos, tan, asin, acos, atan, exp, ln and abs"));
				}
			}
			// Term read, read operation
			if (string[0] == ' ') {
				// Do nothing, breaks are just for readability
			} else if (string[0] == '+') {
				append(term, '+');
			} else if (string[0] == '-') {
				append(term, '+');
				minus++;
			}
			else if (string[0] == '*') append(term, '*');
			else if (string[0] == '/') {
				append(term, '*');
				gonnaDivide = true;
			} else if (string[0] == '^') append(term, '^');
			else break;
		}
	//	std::cerr << "Before end, ";
	//	if (term) std::cerr << " term " << term.print(vars) << " ";
	//	if (added) std::cerr << " added " << added.print(vars) << " ";
	//	if (multiplied) std::cerr << " multiplied " << multiplied.print(vars) << " ";
	//	if (powered) std::cerr << " " << powered.print(vars) << " ";
		formula<scalar> result;
		if (term) {
			if (gonnaDivide) {
				// This occurs when the last operation was division, is just the same as multiplication by the inverse of the number
				gonnaDivide = false;
				term = formula<scalar>::inverse(term);
			}
			if (powered) {
				if (multiplied) {
					if (minus % 2)
						multiplied = formula<scalar>::multiplication(multiplied, minusOne);
					if (added)
						result = formula<scalar>::sum(added, formula<scalar>::multiplication(
											multiplied, formula<scalar>::power(powered, term)));
					else result = formula<scalar>::multiplication(multiplied,
														formula<scalar>::power(powered, term));
				} else {
					if (added) {
						if (minus % 2)
							result = formula<scalar>::sum(added, formula<scalar>::multiplication(
												formula<scalar>::power(powered, term), minusOne));
						else result = formula<scalar>::sum(added, formula<scalar>::power(powered, term));
					}
					else {
						if (minus % 2) result = formula<scalar>::multiplication(
									formula<scalar>::power(powered, term), minusOne);
						else result = formula<scalar>::power(powered, term);
					}
				}
			} else {
				if (multiplied) {
					if (minus % 2)
						multiplied = formula<scalar>::multiplication(multiplied, minusOne);
					if (added) result = formula<scalar>::sum(added,
											formula<scalar>::multiplication(multiplied, term));
					else result = formula<scalar>::multiplication(multiplied, term);
				} else {
					if (added) {
						if (minus % 2) result = formula<scalar>::sum(
									formula<scalar>::multiplication(term, minusOne), added);
						else result = formula<scalar>::sum(added, term);
					} else result = term;
				}
			}
		} else result = formula<scalar>::constant(0);
		//std::cerr << "Parsed formula: " << result.print(vars) << std::endl;
		return result;
	}
//	static inline formula<scalar> custom(scalar (*func)(scalar, scalar)) {

//	}

private:

	inline bool isConstant() const { return (data[0].type == CONSTANT); }
	inline const FORMULA_FLOATING_TYPE& getConstant() const { return data[0].value; }
	inline FORMULA_FLOATING_TYPE& getConstant() { return data[0].value; }

	static inline formula genericUnary(formulaType op, const formula& term) {
		if (term.isConstant()) {
			switch(op) {
			case INVERSE :
				return formula<scalar>(1 / term.getConstant());
			case SQUARE :
				return formula<scalar>(term.getConstant() * term.getConstant());
			case CUBE :
				return formula<scalar>(term.getConstant() * term.getConstant() * term.getConstant());
			case SQUARE_ROOT :
				return formula<scalar>(sqrt(term.getConstant()));
			case SINE :
				return formula<scalar>(sin(term.getConstant()));
			case ARCSINE :
				return formula<scalar>(asin(term.getConstant()));
			case COSINE :
				return formula<scalar>(cos(term.getConstant()));
			case ARCCOSINE :
				return formula<scalar>(acos(term.getConstant()));
			case TANGENT :
				return formula<scalar>(tan(term.getConstant()));
			case ARCTANGENT :
				return formula<scalar>(atan(term.getConstant()));
			case NATURAL_LOGARITHM :
				return formula<scalar>(log(term.getConstant()));
			case ABSOLUTE_VALUE :
				return formula<scalar>(fabs(term.getConstant()));
			default:
				throw(std::logic_error("Undefined operation in formula::genericUnary"));
			}
		}
		std::vector<subFormula> made(1 + term.data.size());
		made[0] = subFormula(op, &made[1]);
		for (unsigned int i = 0; i < term.data.size(); i++) {
			made[i + 1] = term.data[i];
			made[i + 1].relocate((long int)&made[i + 1] - (long int)&term.data[i]);
		}
		return formula<scalar>(made);
	}
	static inline formula genericBinary(formulaType op, formulaType opConst,
										const formula& term1, const formula& term2) {
		if (term1.isConstant()) {
			if (term2.isConstant()) {
				switch(op) {
				case SUM :
					return formula<scalar>(term1.getConstant() + term2.getConstant());
				case MULTIPLICATION :
					return formula<scalar>(term1.getConstant() * term2.getConstant());
				case POWER : // Note that power has its own implementation for a part of this
					return formula<scalar>(pow(term1.getConstant(), term2.getConstant()));
				default:
					throw(std::logic_error("Undefined operation in formula::genericBinary"));
				}
			} else {
				std::vector<subFormula> made(1 + term2.data.size());
				made[0] = subFormula(opConst, term1.getConstant(), &made[1]);
				for (unsigned int i = 0; i < term2.data.size(); i++) {
					made[i + 1] = term2.data[i];
					made[i + 1].relocate((long int)&made[i + 1] - (long int)&term2.data[i]);
				}
				return formula<scalar>(made);
			}
		}
		if (term2.isConstant()) {
			std::vector<subFormula> made(1 + term1.data.size());
			made[0] = subFormula(opConst, term2.getConstant(), &made[1]);
			for (unsigned int i = 0; i < term1.data.size(); i++) {
				made[i + 1] = term1.data[i];
				made[i + 1].relocate((long int)&made[i + 1] - (long int)&term1.data[i]);
			}
			return formula<scalar>(made);
		}
		return fullBinary(op, term1, term2);
	}
	static inline formula fullBinary(formulaType op, const formula& term1, const formula& term2) {
		std::vector<subFormula> made(1 + term1.data.size() + term2.data.size());
		made[0] = subFormula(op, &made[1], &made[1 + term1.data.size()]);
		for (unsigned int i = 0; i < term1.data.size(); i++) {
			made[i + 1] = term1.data[i];
			made[i + 1].relocate((long int)&made[i + 1] - (long int)&term1.data[i]);
		}
		for (unsigned int i = 0; i < term2.data.size(); i++) {
			int pos = i + term1.data.size() + 1;
			made[pos] = term2.data[i];
			made[pos].relocate((long int)&made[pos] - (long int)&term2.data[i]);
		}
		return formula<scalar>(made);
	}

	struct subFormula {
		formulaType type;
		union {
			struct {
				FORMULA_FLOATING_TYPE value;
			};
			struct {
				subFormula* term1;
				subFormula* term2;
			};
			struct {
				FORMULA_FLOATING_TYPE constant;
				subFormula* nonConstant;
			};
			struct {
				subFormula* term;
			};
			struct {
				int index;
			};
			struct {
				scalar (*funcEntry)(const scalar*);
			};
			struct {
				scalar (*funcUnary)(scalar);
				subFormula* arg;
			};
//			struct {
//				scalar (*funcBinary)(scalar, scalar);
//				subFormula* arg1;
//				subFormula* arg2;
//			};
		};
		inline subFormula() {}
		inline subFormula(FORMULA_FLOATING_TYPE val) : type(CONSTANT), value(val) { }
		inline subFormula(formulaType op, int ind) : type(op), index(ind) {
			if (op != CALL)
				throw(std::logic_error("Wrong constructor, to create a constant, use the unary one"));
		}
		inline subFormula(formulaType op, subFormula* t1, subFormula* t2)
			: type(op), term1(t1), term2(t2) { }
		inline subFormula(formulaType op, FORMULA_FLOATING_TYPE c, subFormula* t)
			: type(op), constant(c), nonConstant(t) { }
		inline subFormula(formulaType op, subFormula* t)
			: type(op), term(t) { }
		inline subFormula(formulaType op, scalar (*entry)(const scalar))
			: type(op), funcEntry(entry) { }
		inline subFormula(formulaType op, scalar (*unary)(scalar), subFormula* from)
			: type(op), funcUnary(unary), arg(from) { }
//		inline subFormula(formulaType op, scalar (*binary)(scalar, scalar), subFormula* from1, subFormula* from2)
//			: type(op), funcBinary(binary), arg1(from1), arg2(from2) { }
		inline void relocate(long int difference) { // New minus old
			auto correct = [&] (subFormula*& moving) -> void {
				moving = (subFormula*)((long int)moving + difference);
			};
			switch(type) {
			case SUM :
			case MULTIPLICATION :
			case POWER :
			case LOGARITHM :
				correct(term1);
				correct(term2);
				break;
			case SUM_CONST :
			case MULTIPLICATION_CONST :
			case POWER_CONSTANT :
			case UNNATURAL_EXPONENTIAL :
				correct(nonConstant);
				break;
			case SQUARE :
			case INVERSE :
			case CUBE :
			case SQUARE_ROOT :
			case SINE :
			case ARCSINE :
			case COSINE :
			case ARCCOSINE :
			case TANGENT :
			case ARCTANGENT :
			case NATURAL_LOGARITHM :
			case ABSOLUTE_VALUE :
				correct(term);
			case FUNC_UNARY :
				correct(arg);
				break;
			default:
				break;
			}
		}

		scalar compute(const scalar* const variables,
					   const std::function<scalar(FORMULA_FLOATING_TYPE)>& valueMaker) const {
			switch (type) {
			case CONSTANT :
				if (std::is_integral<scalar>::value) {
					return (scalar)value;
				} else {
					return valueMaker(value);
				}
			case CALL :
				return variables[index];
			case SUM :
				return term1->compute(variables, valueMaker)
						+ term2->compute(variables, valueMaker);
			case SUM_CONST :
				return constant + nonConstant->compute(variables, valueMaker);
			case MULTIPLICATION :
				return term1->compute(variables, valueMaker)
						* term2->compute(variables, valueMaker);
			case MULTIPLICATION_CONST :
				return constant * nonConstant->compute(variables, valueMaker);
			case POWER :
				return pow(term1->compute(variables, valueMaker),
						   term2->compute(variables, valueMaker));
			case SQUARE : {
				scalar got = term->compute(variables, valueMaker);
				return got * got;
			}
			case CUBE : {
				scalar got = term->compute(variables, valueMaker);
				return got * got * got;
			}
			case INVERSE :
				return (FORMULA_FLOATING_TYPE)1.0 / term->compute(variables, valueMaker);
			case SQUARE_ROOT :
				return sqrt(term->compute(variables, valueMaker));
			case POWER_CONSTANT :
				return pow(nonConstant->compute(variables, valueMaker), constant);
			case UNNATURAL_EXPONENTIAL :
				return pow(constant, nonConstant->compute(variables, valueMaker));
			case EXPONENTIAL :
				return exp(term->compute(variables, valueMaker));
			case SINE :
				return sin(term->compute(variables, valueMaker));
			case ARCSINE :
				return asin(term->compute(variables, valueMaker));
			case COSINE :
				return cos(term->compute(variables, valueMaker));
			case ARCCOSINE :
				return acos(term->compute(variables, valueMaker));
			case TANGENT :
				return tan(term->compute(variables, valueMaker));
			case ARCTANGENT :
				return atan(term->compute(variables, valueMaker));
			case LOGARITHM :
				return log(term1->compute(variables, valueMaker))
						/ log(term2->compute(variables, valueMaker));
			case NATURAL_LOGARITHM :
				return log(term->compute(variables, valueMaker));
			case ABSOLUTE_VALUE :
				return abs(term->compute(variables, valueMaker));
			case FUNC_ENTRY :
				return (*funcEntry)(variables);
			case FUNC_UNARY :
				return (*funcUnary)(arg->compute(variables, valueMaker));
//			case FUNC_BINARY :
//				return (*funcBinary)(arg1, arg2);
			default:
				throw(std::logic_error("Function has encountered a really weird operation name"));
			}
		}
		std::string print(const std::vector<std::string>& vars,
				const std::vector<std::pair<std::string, scalar (*)(scalar)>>& unaryFuncs =
				std::vector<std::pair<std::string, scalar (*)(scalar)>>()) const {
			switch (type) {
			case CONSTANT :
				return to_string(value);
			case CALL :
				return "$" + vars[index];
			case SUM :
				return "(" + term1->print(vars, unaryFuncs)
						+ " + " + term2->print(vars, unaryFuncs) + ")";
			case SUM_CONST :
				return "(" + to_string(constant)
						+ " + " + nonConstant->print(vars, unaryFuncs) + ")";
			case MULTIPLICATION :
				return "(" + term1->print(vars, unaryFuncs) + " * " + term2->print(vars, unaryFuncs) + ")";
			case MULTIPLICATION_CONST :
				return "(" + to_string(constant)
						+ " * " + nonConstant->print(vars, unaryFuncs) + ")";
			case POWER :
				return "(" + term1->print(vars, unaryFuncs) + " ^ " + term2->print(vars, unaryFuncs) + ")";
			case SQUARE :
				return "(" + term->print(vars, unaryFuncs) + " ^ 2)";
			case CUBE :
				return "(" + term->print(vars, unaryFuncs) + " ^ 3)";
			case INVERSE :
				return "(1 / " + term->print(vars, unaryFuncs) + ")";
			case SQUARE_ROOT :
				return "sqrt(" + term->print(vars, unaryFuncs) + ")";
			case POWER_CONSTANT :
				return "(" + nonConstant->print(vars, unaryFuncs)
						+ " ^ " + to_string(constant) + ")";
			case UNNATURAL_EXPONENTIAL :
				return "(" + to_string(constant)
						+ " ^ " + nonConstant->print(vars, unaryFuncs) + ")";
			case EXPONENTIAL :
				return "e ^(" + term->print(vars, unaryFuncs) + ")";
			case SINE :
				return "sin(" + term->print(vars, unaryFuncs) + ")";
			case ARCSINE :
				return "arcsin(" + term->print(vars, unaryFuncs) + ")";
			case COSINE :
				return "cos(" + term->print(vars, unaryFuncs) + ")";
			case ARCCOSINE :
				return "arccos(" + term->print(vars, unaryFuncs) + ")";
			case TANGENT :
				return "tan(" + term->print(vars, unaryFuncs) + ")";
			case ARCTANGENT :
				return "arctan(" + term->print(vars, unaryFuncs) + ")";
			case LOGARITHM :
				return "log(" + term1->print(vars, unaryFuncs) + ")(" + term2->print(vars, unaryFuncs) + ")";
			case NATURAL_LOGARITHM :
				return "ln(" + term->print(vars, unaryFuncs) + ")";
			case ABSOLUTE_VALUE :
				return "abs(" + term->print(vars, unaryFuncs) + ")";
			case FUNC_ENTRY :
				return "func_" + to_string((long int)funcEntry) + "";
			case FUNC_UNARY :
				for (int index = 0; index < (int)unaryFuncs.size(); index++)
					if (unaryFuncs[index].second == funcUnary)
						return unaryFuncs[index].first + "(" + arg->print(vars, unaryFuncs) + ")";
				throw(std::logic_error("Found unnamed function as unaryFunction"));
//			case FUNC_BINARY :
//				return "func_" + to_string((long int)funcBinary)
//				+ "(" + arg1->print(vars, unaryFuncs) + ", " + arg2->print(vars, unaryFuncs) + ")";
			default:
				throw(std::logic_error("Function has been given a really weird operation name "
									   + to_string((int)type)));
			}
		}
	};

	std::vector<subFormula> data;
	mutable std::function<scalar(FORMULA_FLOATING_TYPE)> valueMaker;

	formula(std::vector<subFormula>& givenData) { data.swap(givenData); }
};


#endif // FORMULA

