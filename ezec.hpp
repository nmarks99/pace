#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "cadef.h"

namespace ezec {

namespace detail {

using MonitorVariant = std::variant<std::monostate, double, int, std::string>;

template <typename T>
std::optional<T> convert(const MonitorVariant& v) {
    if (auto* val = std::get_if<T>(&v)) {
        return *val;
    }
    if constexpr (std::is_arithmetic_v<T>) {
        if (auto* d = std::get_if<double>(&v)) {
            return static_cast<T>(*d);
        }
        if (auto* i = std::get_if<int>(&v)) {
            return static_cast<T>(*i);
        }
    }
    if constexpr (std::is_same_v<T, std::string>) {
        if (auto* d = std::get_if<double>(&v)) {
            return std::to_string(*d);
        }
        if (auto* i = std::get_if<int>(&v)) {
            return std::to_string(*i);
        }
    }
    return std::nullopt;
}

struct MonitorSlotBase {
    virtual ~MonitorSlotBase() = default;
    virtual void publish(const MonitorVariant& staged) = 0;
};

template <typename T>
struct MonitorSlot : MonitorSlotBase {
    std::vector<T*> targets;
    void publish(const MonitorVariant& staged) override {
        if (auto val = convert<T>(staged)) {
            for (T* t : targets) {
                *t = *val;
            }
        }
    }
};

} // namespace detail

class ChannelBase {
  public:
    ChannelBase(const std::string& pv_name) : pv_name_(pv_name) {}
    virtual ~ChannelBase() = default;

    virtual bool connected() const = 0;

    template <typename T>
    void bind(T& var) {
        std::lock_guard lock(mutex_);
        for (auto& slot : slots_) {
            if (auto* typed = dynamic_cast<detail::MonitorSlot<T>*>(slot.get())) {
                typed->targets.push_back(&var);
                return;
            }
        }
        auto slot = std::make_unique<detail::MonitorSlot<T>>();
        slot->targets.push_back(&var);
        slots_.push_back(std::move(slot));
    }

    bool sync() {
        if (!new_data_.load(std::memory_order_acquire)) {
            return false;
        }
        std::lock_guard lock(mutex_);
        for (auto& slot : slots_) {
            slot->publish(staged_value_);
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
};

class CAChannel : public ChannelBase {
  public:
    CAChannel(const std::string& pv_name) : ChannelBase(pv_name) {
        SEVCHK(ca_create_channel(pv_name.c_str(), connection_callback, this,
                                 CA_PRIORITY_DEFAULT, &channel_id_),
               "ca_create_channel");
        SEVCHK(ca_flush_io(), "ca_flush_io");
    }

    ~CAChannel() {
        if (evt_id_) {
            ca_clear_subscription(evt_id_);
        }
        ca_clear_channel(channel_id_);
    }

    bool connected() const override {
        return connected_.load(std::memory_order_relaxed);
    }

  private:
    chid channel_id_;
    evid evt_id_ = nullptr;
    std::atomic<bool> connected_{false};

    void start_monitor() {
        if (evt_id_) {
            ca_clear_subscription(evt_id_);
            evt_id_ = nullptr;
        }

        auto native = ca_field_type(channel_id_);
        chtype dbr;
        if (native == DBF_STRING) {
            dbr = DBR_STRING;
        } else if (native == DBF_ENUM) {
            dbr = DBR_LONG;
        } else {
            dbr = DBR_DOUBLE;
        }

        SEVCHK(ca_create_subscription(dbr, 1, channel_id_, DBE_VALUE | DBE_ALARM,
                                      subscription_callback, this, &evt_id_),
               "ca_create_subscription");
        SEVCHK(ca_flush_io(), "ca_flush_io");
    }

    static void connection_callback(struct connection_handler_args args) {
        auto* self = static_cast<CAChannel*>(ca_puser(args.chid));
        if (args.op == CA_OP_CONN_UP) {
            self->connected_.store(true, std::memory_order_relaxed);
            self->start_monitor();
        } else {
            self->connected_.store(false, std::memory_order_relaxed);
        }
    }

    static void subscription_callback(struct event_handler_args evt) {
        auto* self = static_cast<CAChannel*>(evt.usr);
        if (evt.status != ECA_NORMAL) {
            return;
        }

        detail::MonitorVariant value;
        short dbr = evt.type;
        if (dbr == DBR_DOUBLE) {
            value = *static_cast<const dbr_double_t*>(evt.dbr);
        } else if (dbr == DBR_LONG) {
            value = static_cast<int>(*static_cast<const dbr_long_t*>(evt.dbr));
        } else if (dbr == DBR_STRING) {
            value = std::string(static_cast<const char*>(evt.dbr));
        }

        std::lock_guard lock(self->mutex_);
        self->staged_value_ = value;
        self->new_data_.store(true, std::memory_order_release);
    }
};

class Context {
  public:
    Context(const std::string& provider = "ca") : provider_(provider) {
        if (provider_ == "ca") {
            SEVCHK(ca_context_create(ca_enable_preemptive_callback), "ca_context_create");
        } else if (provider_ == "pva") {
            throw std::runtime_error("pva not yet implemented");
        } else {
            throw std::runtime_error("Unknown provider " + provider);
        }
    }
    ~Context() {
        if (provider_ == "ca") {
            ca_context_destroy();
        } else if (provider_ == "pva") {
            // TODO: destroy pva context
        }
    }

  private:
    std::string provider_;
};

} // namespace ezec
