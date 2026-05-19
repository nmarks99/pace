#pragma once
#include <atomic>
#include <iomanip>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "cadef.h"

namespace ezec {

namespace detail {

/// \brief std::variant type that holds the latest value from an EPICS subscription.
///
/// PVs that cannot be represented as one of the types in this variant are
/// unsupported.
using MonitorVariant = std::variant<std::monostate, double, int, std::string>;

/// \brief Convert a MonitorVariant to a target type T.
///
/// The MonitorVariant holds the latest value from a CA subscription callback
/// This function attempts to convert it to the type T of the user's bound
/// variable. Returns std::nullopt if the conversion is not supported
template <typename T>
std::optional<T> convert(const MonitorVariant& v, int precision = 4) {

    // Target type T is the same as the variant's type
    if (auto* val = std::get_if<T>(&v)) {
        return *val;
    }

    // Target type T and variant are different, but
    // both arithmetic types, so we cast.
    if constexpr (std::is_arithmetic_v<T>) {
        if (auto* d = std::get_if<double>(&v)) {
            return static_cast<T>(*d);
        }
        if (auto* i = std::get_if<int>(&v)) {
            return static_cast<T>(*i);
        }
    }

    // Target type T is string, and variant type is not
    // so we convert to string if we can.
    if constexpr (std::is_same_v<T, std::string>) {
        if (auto* d = std::get_if<double>(&v)) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << *d;
            return oss.str();
        }
        if (auto* i = std::get_if<int>(&v)) {
            return std::to_string(*i);
        }
    }

    return std::nullopt;
}

/// \brief Type-erased base class for monitor fan-out slots.
///
/// ChannelBase stores a vector of MonitorSlotBase pointers to fan out a single
/// staged MonitorVariant to bound variables of potentially different types.
/// Each concrete MonitorSlot<T> handles conversion and distribution for one
/// target type. This provides type erasure so that ChannelBase doesn't need to
/// know the types of the user's bound variables.
///
/// Slots are created on demand by bind<T>():
/// - If a MonitorSlot<T> already exists, the new variable pointer is appended
///   to its targets vector.
/// - Otherwise, a new MonitorSlot<T> is created.
///
/// When sync() runs, it iterates all slots and calls copy_to_targets() on each.
/// Each slot independently converts the variant to its type T via convert<T>()
/// and writes the result to all of its target pointers.
struct MonitorSlotBase {
    virtual ~MonitorSlotBase() = default;
    virtual void copy_to_targets(const MonitorVariant& staged, int precision = 4) = 0;
};

/// \brief Concrete slot that fans out a MonitorVariant to bound variables of type T.
///
/// Holds raw pointers to the user's bound variables. When copy_to_targets() is
/// called, it converts the staged value via convert<T>() and writes to every
/// target. If conversion fails (e.g. string variant to double target), the
/// targets are left unchanged.
template <typename T>
struct MonitorSlot : MonitorSlotBase {
    std::vector<T*> targets;
    void copy_to_targets(const MonitorVariant& staged, int precision = 4) override {
        if (auto val = convert<T>(staged, precision)) {
            for (T* t : targets) {
                *t = *val;
            }
        }
    }
};

} // namespace detail

/// \brief Abstract base class for a channel bound to a single PV.
///
/// Provides the bind/sync mechanism that decouples the network callback thread
/// from the user's polling thread. Subclasses (e.g. CAChannel) are responsible
/// for connecting to the PV and writing into staged_value_ when new data
/// arrives. The user calls sync() to copy the staged value to all bound
/// variables.
class ChannelBase {
  public:
    ChannelBase(const std::string& pv_name) : pv_name_(pv_name) {}
    virtual ~ChannelBase() = default;

    /// \brief Returns true if the channel is currently connected to the PV.
    virtual bool connected() const = 0;

    /// \brief Bind a user variable to receive monitor updates from this channel.
    ///
    /// The variable will be updated with the latest PV value each time sync() is
    /// called. Multiple variables can be bound to the same channel, including
    /// variables of different types. The bound variable must outlive the channel.
    ///
    /// \param var Reference to the user's variable. A raw pointer to this
    ///            variable is stored internally.
    template <typename T>
    void bind(T& var) {
        std::lock_guard lock(mutex_);
        // If a MonitorSlot for this T already exists,
        // store this pointer in its target vector
        for (auto& slot : slots_) {
            if (auto* typed = dynamic_cast<detail::MonitorSlot<T>*>(slot.get())) {
                typed->targets.push_back(&var);
                return;
            }
        }
        // Create a new slot if no slot for T exists already
        auto slot = std::make_unique<detail::MonitorSlot<T>>();
        slot->targets.push_back(&var);
        slots_.push_back(std::move(slot));
    }

    /// \brief Copy the latest staged value to all bound variables.
    ///
    /// Returns true if new data was available since the last call to sync().
    /// This is the user's polling point. Call sync() periodically from your
    /// application loop.
    bool sync() {
        if (!new_data_.load(std::memory_order_acquire)) {
            return false;
        }
        std::lock_guard lock(mutex_);
        for (auto& slot : slots_) {
            slot->copy_to_targets(staged_value_, precision_);
        }
        new_data_.store(false, std::memory_order_relaxed);
        return true;
    }

  protected:
    std::string pv_name_;
    std::mutex mutex_;
    std::atomic<bool> new_data_{false};
    detail::MonitorVariant staged_value_;
    std::vector<std::unique_ptr<detail::MonitorSlotBase>> slots_;
    int precision_ = 4;
};

/// \brief Helper class for creation/destruction of EPICS (CA/PVA) context
class Context {
  public:
    Context() { SEVCHK(ca_context_create(ca_enable_preemptive_callback), "ca_context_create"); }
    ~Context() { ca_context_destroy(); }
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
};

/// \brief Channel Access implementation of ChannelBase.
///
/// Connects to a single PV via the EPICS Channel Access protocol. A CA context
/// (e.g. ezec::Context) must exist before construction. On connection, a
/// monitor subscription is created automatically and the channel begins
/// receiving value updates into the staged_value_. For floating-point PVs,
/// the record's PREC field is fetched at connection time.
///
/// For write operations, use id() to obtain the raw CA channel identifier
/// and call ca_put() directly.
class CAChannel : public ChannelBase {
  public:
    CAChannel(const std::string& pv_name) : ChannelBase(pv_name) {
        if (!ca_current_context()) {
            throw std::runtime_error("No CA context. Call ca_context_create() before creating a CAChannel.");
        }
        SEVCHK(
            ca_create_channel(pv_name.c_str(), connection_callback, this, CA_PRIORITY_DEFAULT, &channel_id_),
            "ca_create_channel");
        SEVCHK(ca_flush_io(), "ca_flush_io");
    }

    ~CAChannel() {
        if (evt_id_) {
            ca_clear_subscription(evt_id_);
        }
        ca_clear_channel(channel_id_);
    }

    bool connected() const override { return connected_.load(std::memory_order_relaxed); }

    /// \brief Returns the underlying CA channel identifier for direct CA API use.
    chid id() const { return channel_id_; }

  private:
    chid channel_id_;
    evid evt_id_ = nullptr;
    std::atomic<bool> connected_{false};

    /// \brief Creates a monitor subscription and fetches precision on connect.
    ///
    /// Called from the connection callback when the channel comes up. Clears
    /// any existing subscription before creating a new one. For DBF_FLOAT and
    /// DBF_DOUBLE PVs, also issues a one-time ca_get_callback to fetch PREC.
    void start_monitor() {
        if (evt_id_) {
            ca_clear_subscription(evt_id_);
            evt_id_ = nullptr;
        }

        auto native = ca_field_type(channel_id_);
        SEVCHK(ca_create_subscription(dbf_type_to_DBR(native), 1, channel_id_, DBE_VALUE | DBE_ALARM,
                                      subscription_callback, this, &evt_id_),
               "ca_create_subscription");

        if (native == DBF_FLOAT || native == DBF_DOUBLE) {
            ca_get_callback(DBR_CTRL_DOUBLE, channel_id_, precision_callback, this);
        }

        SEVCHK(ca_flush_io(), "ca_flush_io");
    }

    /// \brief CA connection state callback. Starts the monitor on connect.
    static void connection_callback(struct connection_handler_args args) {
        auto* self = static_cast<CAChannel*>(ca_puser(args.chid));
        if (args.op == CA_OP_CONN_UP) {
            self->connected_.store(true, std::memory_order_relaxed);
            self->start_monitor();
        } else {
            self->connected_.store(false, std::memory_order_relaxed);
        }
    }

    /// \brief One-time ca_get_callback handler that extracts PREC from DBR_CTRL_DOUBLE.
    /// Falls back to a default precision of 4 on failure.
    static void precision_callback(struct event_handler_args evt) {
        auto* self = static_cast<CAChannel*>(evt.usr);
        std::lock_guard lock(self->mutex_);
        if (evt.status == ECA_NORMAL) {
            auto* ctrl = static_cast<const struct dbr_ctrl_double*>(evt.dbr);
            self->precision_ = ctrl->precision;
        } else {
            self->precision_ = 4;
        }
    }

    /// \brief CA subscription callback. Converts the incoming value to a
    /// MonitorVariant and stages it for the next sync() call.
    static void subscription_callback(struct event_handler_args evt) {
        auto* self = static_cast<CAChannel*>(evt.usr);
        if (evt.status != ECA_NORMAL) {
            return;
        }

        detail::MonitorVariant value;
        switch (evt.type) {
        case DBR_DOUBLE:
            value = *static_cast<const dbr_double_t*>(evt.dbr);
            break;
        case DBR_FLOAT:
            value = static_cast<double>(*static_cast<const dbr_float_t*>(evt.dbr));
            break;
        case DBR_LONG:
            value = static_cast<int>(*static_cast<const dbr_long_t*>(evt.dbr));
            break;
        case DBR_SHORT:
            value = static_cast<int>(*static_cast<const dbr_short_t*>(evt.dbr));
            break;
        case DBR_CHAR:
            value = static_cast<int>(*static_cast<const dbr_char_t*>(evt.dbr));
            break;
        case DBR_ENUM:
            value = static_cast<int>(*static_cast<const dbr_enum_t*>(evt.dbr));
            break;
        case DBR_STRING:
            value = std::string(static_cast<const char*>(evt.dbr));
            break;
        default:
            return;
        }

        std::lock_guard lock(self->mutex_);
        self->staged_value_ = value;
        self->new_data_.store(true, std::memory_order_release);
    }
};

class ChannelGroup {
  public:
    void add(const std::string& pv_name) {
        std::lock_guard lock(mutex_);
        if (channel_map_.count(pv_name) == 0) {
            channel_map_.emplace(pv_name, std::make_unique<CAChannel>(pv_name));
        }
    }

    bool sync() {
        std::lock_guard lock(mutex_);
        bool new_data = false;
        for (auto& [pv_name, channel] : channel_map_) {
            if (channel->sync()) {
                new_data = true;
            }
        }
        return new_data;
    }

    template <typename T>
    void bind(T& var, const std::string& pv_name) {
        std::lock_guard lock(mutex_);
        get_channel_unlocked(pv_name).bind(var);
    }

    ChannelBase& get_channel(const std::string& pv_name) {
        std::lock_guard lock(mutex_);
        return get_channel_unlocked(pv_name);
    }

  private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<ChannelBase>> channel_map_;

    ChannelBase& get_channel_unlocked(const std::string& pv_name) {
        auto it = channel_map_.find(pv_name);
        if (it == channel_map_.end()) {
            throw std::runtime_error(pv_name + " not registered");
        }
        return *it->second;
    }
};

} // namespace ezec
