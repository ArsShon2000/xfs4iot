#pragma once
#include <type_traits>

namespace XFS4IoT
{
    template<typename TEnum>
    constexpr bool HasFlag(TEnum value, TEnum flag)
    {
        static_assert(std::is_enum_v<TEnum>, "HasFlag requires enum type");
        using Underlying = std::underlying_type_t<TEnum>;
        return (static_cast<Underlying>(value) & static_cast<Underlying>(flag)) != 0;
    }

    template<typename TEnum>
    constexpr bool HasFlag(const std::optional<TEnum>& value, std::optional<TEnum> flag)
    {
        static_assert(std::is_enum_v<TEnum>, "HasFlag requires enum type");
        return value.has_value() && flag.has_value() && HasFlag(value.value(), flag.value());
    }
}

// USING 
// раньше было 
//usage =
//std::make_shared<XFS4IoT::CashAcceptor::PosCapsClass::UsageClass>(
//    EnumHasFlag(
//        value.usage,
//        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::In),
//    EnumHasFlag(
//        value.usage,
//        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Refuse),
//    EnumHasFlag(
//        value.usage,
//        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Rollback));

// а стало
//usage =
//std::make_shared<XFS4IoT::CashAcceptor::PosCapsClass::UsageClass>(
//    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
//        value.usage,
//        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::In),
//    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
//        value.usage,
//        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Refuse),
//    XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::HasFlag(
//        value.usage,
//        XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Rollback));

//Для каждого энума не пишем каст к андерлайну, а используем универсальную функцию HasFlag, которая работает с любым энамом.В C# это было расширяющим методом для всех энамов, в C++ это просто шаблонная функция.