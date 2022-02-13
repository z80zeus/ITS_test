#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "concordance.h"

using namespace std;
using namespace ist;

const string_view srcFilenameParam = "if";    // Название параметра командной строки: "имя входного файла".
const string_view dstFilenameParam = "of";    // Название параметра командной строки: "имя выходного файла".
const string_view sortFieldParam = "field";   // Название параметра командной строки: "поле сортировки".
const string_view sortModeParam = "sort";     // Название параметра командной строки: "режим сортировки"
const string_view wordsIgnoreParam = "ignore";// Название параметра командной строки: "игнорируемые слова"
const char paramDelim = '=';                  // Разделитель названия и значения параметра командной строки.

// Параметры командной строки собираются в хэш-мап, обеспечивающий константное время доступа.
using commandLineParameters = unordered_map<string_view, string_view>;

/**
 * @brief Вывод информации об использовании программы в консоль.
 */
void
showUsage() {
  cout << "Usage:\n" <<
       "IST_test " <<
       "<" << sortFieldParam << "=<word|count|fstPosition|avgDistance>> " <<
       "<" << sortModeParam << "=<asc|desc>> " <<
       "[" << srcFilenameParam << "=inputFileName] " <<
       "[" << dstFilenameParam << "=outputFileName] " <<
       "[" << wordsIgnoreParam << "=<\"word1[ word2[ word3[...]]]\">]" << endl;
}

/**
 * @brief Сервисный оператор: вывод параметров командной строки в поток.
 * @param os Выходной поток.
 * @param params Контейнер с параметрами командной строки.
 * @return Выходной поток.
 */
ostream &
operator<<(ostream& os, commandLineParameters params) {
  os << "CommandLineParameters: " << endl;
  for_each(params.begin(), params.end(), [&os](const auto& param){
    os << param.first << ":" << param.second << endl;
  });

  return os;
}

/**
 * @brief Разбор командной строки в хэш-контейнер.
 * @param argc Количество параметров командной строки.
 * @param argv Массив параметров командной строки.
 * @return Хэш-контейнер с параметрами командной строки.
 */
auto
parseCommandLineParameters(int argc, const char* argv[]) noexcept(false) {
  commandLineParameters params;

  for_each_n(argv, argc, [&](auto p) { // Обработка всех параметров командной строки: O(N)
    string_view param(p);
    auto delimIndex = param.find(paramDelim);
    auto paramName = param.substr(0, delimIndex);
    auto paramValue = delimIndex != string::npos ? param.substr(delimIndex + 1) : "";
    params[paramName] = paramValue; // Вставка в хэш-таблицу: близко к O(1) если не произойдёт рехэширования.
  });

  return params;
}

/**
 * @brief Создать файловый поток исходных данных.
 * @param params Параметры командной строки. Оттуда забирается параметр if.
 * @return Файловый поток исходных данных. Если в командной строке нет параметра if, то возвращаемый файловый поток не будет открыт.
 * @throw invalid_argument Не удалось открыть файловый поток.
 */
auto
createInputFile(const auto& params) noexcept(false){
  ifstream inputFile;
  if (params.contains(srcFilenameParam)) {  // Поиск в хэш-таблице: близко к O(1)
    inputFile.open(params.at(srcFilenameParam).data());
    if (!inputFile) throw invalid_argument(string("Can't open file ") + params.at(srcFilenameParam).data());
  }
  return inputFile;
}

/**
 * @brief Создать файловый поток выходных данных.
 * @param params Параметры командной строки. Оттуда забирается параметр of.
 * @return Файловый поток выходных данных. Если в командной строке нет параметра of, то возвращаемый файловый поток не будет открыт.
 * @throw invalid_argument Не удалось открыть файловый поток.
 */
auto
createOutputFile(const auto& params) noexcept(false){
  ofstream outputFile;
  if (params.contains(dstFilenameParam)) {
    outputFile.open(params.at(dstFilenameParam).data());
    if (!outputFile) throw invalid_argument(string("Can't open file ") + params.at(dstFilenameParam).data());
  }
  return outputFile;
}

/**
 * @brief Проверка корректности параметров командной строки. При обнаружении некорректного параметра - выбрасывает исключение.
 * @param params Контейнер с параметрами командной строки.
 * @throw invalid_argument - ошибка в параметрах.
 */
void
checkParams(const auto& params) noexcept(false){
  if (!params.contains(sortFieldParam))
    throw invalid_argument(string("No required param ") + sortFieldParam.data());

  if (!params.contains(sortModeParam))
    throw invalid_argument(string("No required param ") + sortFieldParam.data());
}

/**
 * @brief Создать контейнер игнорируемых слов перечисленных в значении параметра командной строки ignore.
 * @param params Контейнер с параметрами командной строки.
 * @return Хэш-сет игнорируемых слов.
 */
auto
createIgnoreWordsSet(auto params) {
  unordered_set<string> ignoreWords;

  if (!params.contains(wordsIgnoreParam)) return ignoreWords;

  istringstream ignoreWordsStream(params[wordsIgnoreParam].data());
  string word;
  while (ignoreWordsStream >> word) {
    // Вставка в хэш-контейнер ~O(1).
    // emplace - создаёт объект внутри контейнера, предотвращая создание временного объекта.
    // move - приводит к перемещающему конструированию строки.
    ignoreWords.emplace(move(word));
  }

  return ignoreWords;
}

int
main(int argc, const char* argv[]) {
  try {
    auto params = parseCommandLineParameters(argc, argv); // O(argc)
    checkParams(params); // O(1)

    auto ignoreWords = createIgnoreWordsSet(params); // O(ignoreWords)

    auto inputFile = createInputFile(params);
    auto& src = inputFile.is_open()? inputFile : cin;

    ofstream outputFile = createOutputFile(params);
    auto& dst = outputFile.is_open()? outputFile : cout;

    auto result = makeConcordance(src, ignoreWords); // O(words)

    sort(result, params[sortFieldParam], params[sortModeParam]); // ~O(NlogN)

    dst << result;

    return 0;
  }
  catch (const invalid_argument& e) {
    cerr << e.what() << endl;
    showUsage();
    return -1;
  }
}
