# tool_cpp_events
Small c++ tools for managing of events

Класс Sender<> рассылает событие во все Receptor<> которые были
к нему привязаны. Receptor может быть как созданный класс, и
может быть как лямбда:

```cpp
Sender<int> sender;
Receptor<int> receptor([](int a) { std::cout << "ev1" << a; });

sender << receptor;
sender << [](int a) { std::cout << "ev2" << a; };
```

В случае, если приемник добавлен как Receptor<>,
то в случае удаления Receptor<> он автоматически
исключится из Sender<>. И наоборот, если удаляется
Sender<> он отключится от Receptor<>;

Так же Sender<> может рассылать и в последующие Sender<>:

```cpp
Sender<int> sender1;
Sender<int> sender2;

sender1 << sender2;
```

B Receptor<> может рассылать в другие Receptor<> аналогичным образом.
Во многом различия Sender и Receptor, в том что это явно обозначенные
где возникает событие, и кто его будет принимать.


Examples of use:

In first object:
```cpp
Sender<int,int> sender1;
```

Somewhere:
```cpp
sender1 << [&](int a, int b) {
  std::cout "event1" << a << b;
};

sender1 << [&](int a, int b) {
  std::cout << "event2" << a << b;
};
```
And when happened:
```cpp
sender1(10, 20);
```
then:
```cpp
event1 10 20
event2 10 20
```



