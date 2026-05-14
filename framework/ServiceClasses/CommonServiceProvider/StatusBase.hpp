#pragma once
#include <string>
#include <functional>


namespace XFS4IoTFramework::Common
{
    class StatusBase
    {
    public:
        virtual ~StatusBase() = default;

    protected:
        void NotifyPropertyChanged(const std::string& propertyName = "")
        {
            if (propertyChangedCallback_) {
                propertyChangedCallback_(propertyName);
            }
        }

        void SetPropertyChangedCallback(std::function<void(const std::string&)> callback)
        {
            propertyChangedCallback_ = std::move(callback);
        }

    private:
        std::function<void(const std::string&)> propertyChangedCallback_;
    };
}