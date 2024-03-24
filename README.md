# Запуск приложения
## Запуск сервера
```
# Компиляция
c++ -std=c++11 phonebook_server.cc phonebook.grpc.pb.cc phonebook.pb.cc -lgrpc++ -lprotobuf -lpthread -ldl -o phonebook_server

# Запуск
./phonebook_server
```

## Запуск клиента
```
python3 phonebook_client.py
```