#include <fstream>
#include <iostream>

#include <gram/language/parser/BnfRuleParser.h>
#include <gram/population/initializer/RandomInitializer.h>
#include <gram/population/selector/TournamentSelector.h>
#include <gram/util/bool_generator/TwisterBoolGenerator.h>
#include <gram/util/number_generator/TwisterNumberGenerator.h>
#include <gram/Evolution.h>

using namespace gram;
using namespace std;

class FakeEvaluator : public Evaluator {
  int evaluate(string program) const {
    return 0;
  }
};

class FakeFitnessCalculator : public FitnessCalculator {
 public:
  double calculate(int desired, int actual) const {
    return abs(desired - actual);
  }
};

string loadFile(string name) {
  ifstream grammarFile(name);

  if (!grammarFile.is_open()) {
    return "";
  }

  string content((istreambuf_iterator<char>(grammarFile)), istreambuf_iterator<char>());

  return content;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return 1;
  }

  string grammarString = loadFile(argv[1]);

  unsigned long max = numeric_limits<unsigned long>::max();
  unique_ptr<NumberGenerator> numberGenerator1 = make_unique<TwisterNumberGenerator>(max);
  unique_ptr<NumberGenerator> numberGenerator2 = make_unique<TwisterNumberGenerator>(29);
  unique_ptr<NumberGenerator> numberGenerator3 = make_unique<TwisterNumberGenerator>(11);
  unique_ptr<NumberGenerator> numberGenerator4 = make_unique<TwisterNumberGenerator>(11);
  unique_ptr<BoolGenerator> boolGenerator = make_unique<TwisterBoolGenerator>(0.1);

  auto selector = make_unique<TournamentSelector>(move(numberGenerator1));
  auto mutation = make_unique<Mutation>(move(boolGenerator), move(numberGenerator2));
  auto crossover = make_unique<Crossover>(move(numberGenerator3));
  auto generator = make_shared<Generator>(move(selector), move(crossover), move(mutation));

  BnfRuleParser parser;

  shared_ptr<Grammar> grammar = parser.parse(grammarString);

  auto mapper = make_unique<Mapper>(grammar);
  auto language = make_shared<Language>(grammar, move(mapper));

  RandomInitializer initializer(move(numberGenerator4), language, 100);

  unique_ptr<Evaluator> evaluator = make_unique<FakeEvaluator>();
  unique_ptr<FitnessCalculator> calculator = make_unique<FakeFitnessCalculator>();
  auto processor = make_unique<Processor>(move(evaluator), move(calculator));

  Evolution evolution(move(processor));

  Population population = initializer.initialize(1000, generator);

  Individual result = evolution.run(population, 1470);

  cout << result.fitness() << " : " << result.serialize() << endl;

  return 0;
}
