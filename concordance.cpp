#include <algorithm>
#include <unordered_map>

#include "concordance.h"

using namespace std;
using namespace ist;

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
sortByWord(concordance &concor, string_view oType) {
  sort(begin(concor), end(concor), [&oType](const auto &wordStat1, const auto wordStat2) {
    if (oType == "asc") return wordStat1.word < wordStat2.word;
    if (oType == "desc") return wordStat1.word > wordStat2.word;
  });
  return concor;
}

concordance &
sortByCount(concordance &concor, string_view oType) {
  sort(begin(concor), end(concor), [&oType](const auto &wordStat1, const auto wordStat2) {
    if (oType == "asc") return wordStat1.count < wordStat2.count;
    if (oType == "desc") return wordStat1.count > wordStat2.count;
  });
  return concor;
}

concordance &
sortByFstPosition(concordance &concor, string_view oType) {
  sort(begin(concor), end(concor), [&oType](const auto &wordStat1, const auto wordStat2) {
    if (oType == "asc") return wordStat1.fstPosition < wordStat2.fstPosition;
    if (oType == "desc") return wordStat1.fstPosition > wordStat2.fstPosition;
  });
  return concor;
}

concordance &
sortByAvgDistance(concordance &concor, string_view oType) {
  sort(begin(concor), end(concor), [&oType](const auto &wordStat1, const auto wordStat2) {
    if (oType == "asc") return wordStat1.avgDistance < wordStat2.avgDistance;
    if (oType == "desc") return wordStat1.avgDistance > wordStat2.avgDistance;
  });
  return concor;
}

//enum class sortType { asc, desc };
//
//sortType
//getSortTypeByName(std::string_view sType) noexcept(false) {
//  if (sType == "asc") return sortType::asc;
//  if (sType == "desc") return sortType::desc;
//  throw invalid_argument(string("Unknown sort type ") + sType.data());
//}
//
//template<typename T>
//T
//getFieldByName(std::string_view fieldName) noexcept(false){
//  if (fieldName == "word") return &wordCharacter::word;
//  if (fieldName == "count") return &wordCharacter::count;
//  if (fieldName == "fstPosition") return &wordCharacter::fstPosition;
//  if (fieldName == "avgDistance") return &wordCharacter::avgDistance;
//  throw invalid_argument(string("Unknown field name ") + fieldName.data());
//}
//
//template<typename Field, class C = wordCharacter>
//concordance&
//sort(concordance& concor, Field C::* field, sortType sType) {
//  sort(begin(concor), end(concor), [&field, &sType](const auto& wordStat1, const auto wordStat2) {
//    switch (sType) {
//      case sortType::asc: return wordStat1.*field < wordStat2.field;
//      case sortType::desc: return wordStat1.field > wordStat2.field;
//    }
//  });
//  return concor;
//}

concordance &
ist::sort(concordance &concor, string_view fieldName, std::string_view sortType) {
//  return sort(concor, getFieldByName(fieldName), getSortTypeByName(sortType));

  if (fieldName == "word") return sortByWord(concor, sortType);
  if (fieldName == "count") return sortByCount(concor, sortType);
  if (fieldName == "fstPosition") return sortByFstPosition(concor, sortType);
  if (fieldName == "avgDistance") return sortByAvgDistance(concor, sortType);
  throw invalid_argument(string("Unknown field to sort:") + fieldName.data());
}
