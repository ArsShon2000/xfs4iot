#pragma once

#include <string>
#include <memory>
#include "../../core/Events/PropertyChangedEventArgs.hpp"

namespace XFS4IoTFramework::Storage
{
    /// <summary>
    /// Base record class for storage-related objects that notify about changes
    /// </summary>
    class StorageChangedBaseRecord : public XFS4IoTServer::NotifyPropertyChangedBase,
        public std::enable_shared_from_this<StorageChangedBaseRecord>
    {
    public:
        virtual ~StorageChangedBaseRecord() = default;

        /// <summary>
        /// Storage ID associated with this record
        /// </summary>
        const std::string& getStorageId() const { return storageId_; }
        void setStorageId(const std::string& id) { storageId_ = id; }

    protected:
        StorageChangedBaseRecord() = default;

        /// <summary>
        /// Helper method to notify about property changes
        /// Calls raisePropertyChanged from base class
        /// </summary>
        void notifyPropertyChanged(const std::string& propertyName = "")
        {
            if (propertyName.empty())
            {
                raisePropertyChanged();
            }
            else
            {
                raisePropertyChanged(propertyName);
            }
        }

    private:
        std::string storageId_;
    };
}