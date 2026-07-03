#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace spqb
{

class HashAlgorithm
{
public:
    virtual ~HashAlgorithm() = default;

    [[nodiscard]]
    virtual std::string name() const noexcept = 0;

    [[nodiscard]]
    virtual std::vector<std::uint8_t>
    hash(std::span<const std::uint8_t> message) = 0;

    [[nodiscard]]
    virtual std::size_t digestSize() const noexcept = 0;
};

using HashPtr = std::unique_ptr<HashAlgorithm>;

} // namespace spqb
