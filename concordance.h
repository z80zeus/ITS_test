#pragma once

#include <istream>
#include <string>
#include <vector>
#include <unordered_set>

namespace ist {

  /**
  * @brief Характеристика для каждого уникального слова текста.
  */
  struct wordCharacter {
    std::string word;        /*!< Слово */
    size_t count = 0;        /*!< Число появлений слова в тексте */
    size_t fstPosition = 0;  /*!< Первая позиция в тексте */
    size_t avgDistance = 0;  /*!< Средняя дистанция между позициями слова в тексте */
  };

  /**
  * @brief Конкорданс, содержащий характеристику текста.
  */
  using concordance = std::vector<wordCharacter>;

  /**
   * @brief Оператор вывода конкорданса в поток.
   * @param os Целевой поток.
   * @param concor Объект для вывода.
   * @return Переданный поток вывода.
   */
  std::ostream &
  operator<<(std::ostream &os, const concordance& concor);

  /**
   * @brief Функция создаёт конкорданс из текста, находящегося в потоке ввода.
   * @param wordRecord Объект, реализующий интерфейс потока ввода.
   * @param ignoreWords Слова, которые игнорируются. Статистика по ним не считается.
   * @return Объект конкорданс, соответствующий поданому функции тексту.
   */
  concordance makeConcordance(std::istream &wordRecord, const std::unordered_set<std::string>& ignoreWords = std::unordered_set<std::string>());

  /**
   * @brief Сортировать конкорданс.
   * @param concor Конкорданс для сортировки.
   * @param fieldName Название поля по которому следует произвести сортировку.
   * @param sortType По возрастанию (asc) или убыванию (desc) сортировать конкорданс.
   * @return Функция возвращает тот же конкорданс, который ей был передан параметром.
   */
  concordance&
  sort(concordance& concor, std::string_view fieldName = "word", std::string_view sortType = "asc");
}