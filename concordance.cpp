#include <functional>
#include <unordered_map>

#include "concordance.h"

using namespace std;
using namespace ist;

/**
 * @brief Обобщённый компаратор "на меньше" любого поля структуры wordCharacter.
 * @tparam FieldType Тип поля по которому будет производиться сортировка. Нужен для описания следующего шаблонного параметра.
 * @tparam Field  Указатель на поле по которому будет производиться сортировка.
 * @param wordStatL "Левая" сравниваемая структура wordCharacter.
 * @param wordStatR "Правая" сравниваемая структура wordCharacter.
 * @return true если поле Field строго меньше у левой структуры. false - в противном случае.
 * После типизации полем, все компараторы имеют одинаковую сигнатуру (wordStatL, wordStatR) и могут быть помещены в
 * качестве функциональных объектов в один контейнер.
 */
template<typename FieldType, FieldType wordCharacter::* Field>
bool
lessComparator(const wordCharacter& wordStatL, const wordCharacter& wordStatR) {
  return wordStatL.*Field < wordStatR.*Field;
}

/**
 * @brief Обобщённый компаратор "на больше" любого поля структуры wordCharacter.
 * @tparam FieldType Тип поля по которому будет производиться сортировка. Нужен для описания следующего шаблонного параметра.
 * @tparam Field  Указатель на поле по которому будет производиться сортировка.
 * @param wordStatL "Левая" сравниваемая структура wordCharacter.
 * @param wordStatR "Правая" сравниваемая структура wordCharacter.
 * @return true если поле Field строго меньше у левой структуры. false - в противном случае.
 * После типизации полем, все компараторы имеют одинаковую сигнатуру (wordStatL, wordStatR) и могут быть помещены в
 * качестве функциональных объектов в один контейнер.
 */
template<typename FieldType, FieldType wordCharacter::* Field>
bool
greaterComparator(const wordCharacter& wordStatL, const wordCharacter& wordStatR) {
  return wordStatL.*Field > wordStatR.*Field;
}

/**
 * Словарь отображения режимов сортировки в соответствующий компаратор.
 */
static unordered_map<string, function<bool(const wordCharacter&,const wordCharacter&)>> comparators ({
    {"word asc",         lessComparator < decltype(wordCharacter::word),       &wordCharacter::word >},
    {"count asc",        lessComparator < decltype(wordCharacter::count), &wordCharacter::count >},
    {"fstPosition asc",  lessComparator < decltype(wordCharacter::fstPosition), &wordCharacter::fstPosition >},
    {"avgDistance asc",  lessComparator < decltype(wordCharacter::avgDistance), &wordCharacter::avgDistance >},

    {"word desc", greaterComparator < decltype(wordCharacter::word),        &wordCharacter::word >},
    {"count desc", greaterComparator < decltype(wordCharacter::count), &wordCharacter::count >},
    {"fstPosition desc", greaterComparator < decltype(wordCharacter::fstPosition), &wordCharacter::fstPosition >},
    {"avgDistance desc", greaterComparator < decltype(wordCharacter::avgDistance), &wordCharacter::avgDistance >},
});

/**
 * @brief Нормализация строки: очистка от знаков пунктуации и преобразование к нижнему регистру.
 * @param word Ссылка на строку, над которой будут производиться преобразования.
 * @return Ссылка на переданную, обработанную строку.
 */
string &
normaliseWord(std::string &word) {
  erase_if(word, ::ispunct);
  transform(cbegin(word), cend(word), begin(word), ::tolower);
  return word;
}

concordance
ist::makeConcordance(istream &is, const unordered_set<string> &ignoreWords) {

  // Хэш-словари обеспечивают константное время поиска/доступа по ключу и добавление нового элемента.
  unordered_map<string, size_t> wordLastIndex; // Словарь: слово -> его последний индекс в тексте.
  unordered_map<string, wordCharacter> concor;  // Словарь: слово -> его статистика.

  size_t currentWordIndex = 0;                  // Индекс текущего слова в потоке
  string word;

  while (is >> word) {
    word = normaliseWord(
        word);                 // Нормализация слова: удаление знаков препинания, приведение к нижнему регистру.
    if (word.empty()) continue;                 // Слово не прошло нормализацию - не обрабатывается.
    if (ignoreWords.contains(word)) continue; // Прочитанное слово в списке игнорируемых.

    if (!concor.contains(word)) {             // Первое появление слова в тексте.
      concor[word] = {word, 1, currentWordIndex}; // Создать для него запись статистики
      wordLastIndex[word] = currentWordIndex;
      ++currentWordIndex;
      continue;
    }
    // Обработка повторного появления слова в потоке:
    auto &wordCharacter = concor[word];
    wordCharacter.count++;
    // При чтении текста в поле avgDistance (средняя дистанция) копится сумма дистанций для данного слова.
    // Среднее же значение будет вычислено из этой суммы по завершении обработки текста.
    wordCharacter.avgDistance += currentWordIndex - wordLastIndex[word];
    wordLastIndex[word] = currentWordIndex;

    ++currentWordIndex;
  }

  // Подготовка возвращаемого значения.
  concordance result;
  result.reserve(concor.size());  // Для предотвращения реалокаций - заранее зарезервировать ёмкость контейнера.

  // Для каждого слова вычисляется среднее значение дистанции. Сумма дистанций была накоплена в поле avg_distance.
  for_each(concor.begin(), concor.end(), [&result](auto &wordRecord) {
    auto wordStat = wordRecord.second;
    if (wordStat.count > 2) wordStat.avgDistance /= wordStat.count - 1;

    result.emplace_back(move(wordStat));  // Статистика по каждому слову переносится в возвращаемый результат.
  });

  return result;
}

ostream &
ist::operator<<(ostream &os, const concordance &concor) {
  for_each(cbegin(concor), cend(concor), [&os](const auto &c) {
    os << "{" <<
       " word:" << c.word << ", " <<
       " count:" << c.count << ", " <<
       "fstPosition:" << c.fstPosition << ", " <<
       "avgDistance:" << c.avgDistance <<
       " }" << endl;
  });
  return os;
}

concordance &
ist::sort(concordance &concor, string_view fieldName, string_view sortType) {

  string comparatorKey = string(fieldName) + " " + string(sortType);

  if (!comparators.contains(comparatorKey))
    throw invalid_argument("No algorithm for sort by " + comparatorKey);

  sort(begin(concor), end(concor), comparators[comparatorKey]);

  return concor;
}
