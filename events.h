
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <vector>
#include <functional>
#include <cassert>

using std::vector;

template <class ...Args> struct Receptor;

template <typename ...Args>
struct Sender {
    friend Receptor<Args...>;

    using Rcp = Receptor<Args...>;
    using Fn = function<void(Args ...)>;

    bool empty() const { return p_list_use.empty(); }
    bool has() const { return !empty(); }

    void operator()(Args ...args) const { send(args...); }

    void send(Args ...args) const {
        for (const auto *e : p_list_use)
            e->run(args...);
    }

    // оператор << действует по подобию
    //   оператора << для списков -
    //   добавляет в свой список рассылок
    //   нового приемника событий

    void operator << (Rcp &rcp) { *this << &rcp; }
    void operator << (Rcp *rcp) { rcp->bind_sender(this); }
    void operator << (const Fn &fn) { append_function(fn); }

    // при таком назначении, это означает что
    //  когда у текущего экземпляра произойдет вызов send,
    //  то он это событие перешлет в Sender *other,
    //  который уже в свою очередь разошлет в свой список получателей
    void operator << (Sender &other) { *this << &other; }
    void operator << (Sender *other) { *this << other->get_receptor(); }

    // добавить лямбду не привязанную к Receptor
    void append_function(const Fn &fn);

    ~Sender();

private:

    // при получении события этим рецептором,
    //   текущий сендер должен его переслать в свой список получателей.
    //   (в нем сидит лямбда, которая вызове send)
    Rcp *p_incoming_event = nullptr;
    Rcp *get_receptor();

    // список получателей
    vector<Rcp*> p_list_use;

    // список получателей, которые были выделены текущим же объектом,
    //   и потому должны быть удалены в деструкторе.
    //   В списке p_list_use они так же добавлены.
    vector<Rcp*> p_list_alloced;
};

template <typename ...Args>
struct Receptor {

    using Sndr = Sender<Args...>;
    using Fn = function<void(Args...)>;

    Fn process;

    void operator()(Args... args) const { run(args...); }

    void run(Args... args) const {
        if (process) process(args...);
        if (p_outgoing_events)
            p_outgoing_events->send(args...);
    }

    void operator << (Receptor &other) { *this << &other; }
    void operator << (Receptor *other) { *get_outgoing() << other; }

    void operator = (const Fn &fn) { process = fn; }

    Receptor() {}
    Receptor(const Fn &fn) : process(fn) {}
    ~Receptor();

    void bind_sender(Sndr &s) { bind_sender(&s); }
    void bind_sender(Sndr *s);
    void unbind_sender(Sndr &s) { unbind_sender(&s); }
    void unbind_sender(Sndr *s);
    void unbind_all_senders();

private:
    vector<Sndr*> p_list_senders;
    Sndr *p_outgoing_events = nullptr;
    Sndr *get_outgoing();
};

//************************************************************

template <typename ...Args>
Sender<Args...>::~Sender() {

    while (p_list_use.empty()==false)
        p_list_use.back()->unbind_sender(this);

    for (auto *p : p_list_alloced)
        delete p;

    if (p_incoming_event)
        delete p_incoming_event;
}

template <typename ...Args>
void Sender<Args...>::append_function(const Fn &fn) {
    Rcp *p = new Rcp(fn);
    p_list_alloced.push_back(p);
    *this << p;
}

template <typename ...Args>
Receptor<Args...> *Sender<Args...>::get_receptor() {
    if (p_incoming_event)
        return p_incoming_event;

    const auto fn = [this](Args ...args) {
        send(args...); };

    p_incoming_event = new Rcp(fn);

    return p_incoming_event;
}

template <typename ...Args>
Receptor<Args...>::~Receptor() {
    unbind_all_senders();
    if (p_outgoing_events)
        delete p_outgoing_events;
}

template <typename ...Args>
Sender<Args...> *Receptor<Args...>::get_outgoing() {

    if (p_outgoing_events)
        return p_outgoing_events;

    p_outgoing_events = new Sender<Args...>;
    return p_outgoing_events;
}

template <typename ...Args>
void Receptor<Args...>::bind_sender(Sndr *s) {

    auto &l1 = p_list_senders;
    if (l1.end() != std::find(l1.begin(), l1.end(), s))
        return;

    p_list_senders.push_back(s);
    s->p_list_use.push_back(this);
}

template <typename ...Args>
void Receptor<Args...>::unbind_all_senders() {
    while (p_list_senders.empty()==false)
        unbind_sender(p_list_senders.back());
}

template <typename ...Args>
void Receptor<Args...>::unbind_sender(Sndr *s) {

    if (!s)
        return;

    auto fn_del = [](auto &list, auto *val) {
        auto itr = std::find(list.begin(), list.end(), val);
        assert( itr != list.end() );
        list.erase(itr);
    };

    fn_del(p_list_senders, s);
    fn_del(s->p_list_use, this);
}


#endif // EVENTS_H
