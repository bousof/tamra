/*
 *
 *  Copyright (c) 2026 Sofiane BOUSABAA
 *  Licensed under the MIT License (see LICENSE file in project root)
 *
 *  Description: Templating of Tree iterators classes.
 * 
 */

#pragma once

#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "HilbertIterator.h"
#include "MortonIterator.h"


//-----------------------------------------------------------//
//  INTERNAL UTILITIES                                       //
//-----------------------------------------------------------//

namespace tamra::detail {
  template<typename CellType>
  using SupportedTreeIterators = std::tuple<
    HilbertIterator<CellType>,
    MortonIterator<CellType, 123>,
    MortonIterator<CellType, 132>,
    MortonIterator<CellType, 213>,
    MortonIterator<CellType, 231>,
    MortonIterator<CellType, 312>,
    MortonIterator<CellType, 321>
  >;
  
  template<typename IteratorType>
  std::string iterator_tag_as_string() {
      const auto& tag = IteratorType::CONFIG_SELECTION_NAME;
      return std::string(tag.begin(), tag.end());
  }

  template<typename IteratorTuple, std::size_t... Is>
  std::string supported_iterator_tags_impl(std::index_sequence<Is...>) {
    std::string error;
    ((error += " \"" + iterator_tag_as_string<std::tuple_element_t<Is, IteratorTuple>>() + "\"," ), ...);

    if (!error.empty())
      error.back() = '.';

    return error;
  }

  template<typename IteratorTuple>
  std::string supported_iterator_tags() {
    return supported_iterator_tags_impl<IteratorTuple>(
      std::make_index_sequence<std::tuple_size_v<IteratorTuple>>{}
    );
  }

  template<typename T>
  struct type_tag {
    using type = T;
  };

  template<typename IteratorTuple, std::size_t I = 0, typename Func>
  decltype(auto) dispatch_on_tree_iterator_impl(const std::string& iterator_type, Func&& f) {
    if constexpr (I < std::tuple_size_v<IteratorTuple>) {
      //-----------------------------------------------------------//
      //  TEMPLATING                                               //
      //-----------------------------------------------------------//
      using IteratorType = std::tuple_element_t<I, IteratorTuple>;

      if (iterator_type == iterator_tag_as_string<IteratorType>())
        return std::forward<Func>(f)(type_tag<IteratorType>{});

      return dispatch_on_tree_iterator_impl<IteratorTuple, I + 1>(iterator_type, std::forward<Func>(f));
    } else {
      //-----------------------------------------------------------//
      //  ERROR MESSAGE                                            //
      //-----------------------------------------------------------//

      std::string error = "Unknown tree iterator type \"" + iterator_type +
                          "\". Possible values are:" + supported_iterator_tags<IteratorTuple>();

      throw std::runtime_error(error);
    }
  }
}


//-----------------------------------------------------------//
//  PUBLIC API                                               //
//-----------------------------------------------------------//

template<typename CellType, typename Func>
decltype(auto) dispatch_on_tree_iterator(const std::string& iterator_type, Func&& f) {
  using IteratorTuple = tamra::detail::SupportedTreeIterators<CellType>;

  return tamra::detail::dispatch_on_tree_iterator_impl<IteratorTuple>(iterator_type, std::forward<Func>(f));
}
