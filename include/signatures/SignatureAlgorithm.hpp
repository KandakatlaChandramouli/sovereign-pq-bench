#pragma once

#include <memory>

#include "crypto/AbstractCryptoInterface.hpp"

namespace spqb
{

using SignaturePtr = std::unique_ptr<AbstractCryptoInterface>;

} // namespace spqb
