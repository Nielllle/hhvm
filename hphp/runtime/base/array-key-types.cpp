/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-key-types.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP {

void ArrayKeyTypes::toJitType(jit::Type& type) const {
  using namespace jit;
  type = TBottom;
  if (m_bits & kNonStaticStrKey) type |= TStr;
  if (m_bits & kStaticStrKey) type |= TStaticStr;
  if (m_bits & kIntKey) type |= TInt;
}

bool ArrayKeyTypes::checkInvariants(const VanillaDict* ad) const {
  DEBUG_ONLY uint8_t true_bits = 0;
  VanillaDictElm* elm = ad->data();
  for (auto const end = elm + ad->iterLimit(); elm < end; elm++) {
    true_bits |= [&]{
      if (elm->isTombstone())        return kTombstoneKey;
      if (elm->hasIntKey())          return kIntKey;
      if (elm->strKey()->isStatic()) return kStaticStrKey;
      else                           return kNonStaticStrKey;
    }();
  }
  DEBUG_ONLY auto const all =
    kTombstoneKey | kIntKey | kStaticStrKey | kNonStaticStrKey;
  assert_flog((true_bits & ~m_bits) == 0,
              "Untracked key type: true = {}, pred = {}\n",
              true_bits, m_bits);
  assertx((m_bits & ~all) == 0);
  return true;
}

std::string ArrayKeyTypes::show() const {
  if (!m_bits) return "Empty";
  auto types = std::vector<std::string>();
  if (m_bits & kNonStaticStrKey) types.push_back("NonStaticStr");
  if (m_bits & kStaticStrKey) types.push_back("StaticStr");
  if (m_bits & kIntKey) types.push_back("Int");
  if (m_bits & kTombstoneKey) types.push_back("Tombstone");
  return folly::join('|', types);
}

} // namespace HPHP
