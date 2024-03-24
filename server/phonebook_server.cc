#include "phonebook.grpc.pb.h"
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <fstream>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;

const std::string DATA_FILE = "contacts.data";

std::unordered_map<int, Contact> contacts;
int next_id = 1;

void loadContactsFromFile() {
    std::ifstream file(DATA_FILE, std::ios::binary);
    if (file.is_open()) {
        Contact contact;
        while (file.read(reinterpret_cast<char*>(&contact), sizeof(Contact))) {
            contacts[contact.id()] = contact;
            next_id = std::max(next_id, contact.id() + 1);
        }
        file.close();
    }
}

void saveContactsToFile() {
    std::ofstream file(DATA_FILE, std::ios::binary);
    if (file.is_open()) {
        for (const auto& pair : contacts) {
            const Contact& contact = pair.second;
            file.write(reinterpret_cast<const char*>(&contact), sizeof(Contact));
        }
        file.close();
    }
}

class PhonebookServiceImpl final : public PhonebookService::Service {
public:
    ::grpc::Status AddContact(::grpc::ServerContext* context, const ::Contact* request, ::Status* response) override;
    ::grpc::Status RemoveContact(::grpc::ServerContext* context, const ::ContactId* request, ::Status* response) override;
    ::grpc::Status SearchContacts(::grpc::ServerContext* context, const ::SearchQuery* request, ::Contacts* response) override;
    ::grpc::Status GetContact(::grpc::ServerContext* context, const ::ContactId* request, ::Contact* response) override;
};

::grpc::Status PhonebookServiceImpl::AddContact(::grpc::ServerContext* context, const ::Contact* request, ::Status* response) {
    Contact new_contact = *request;
    new_contact.set_id(next_id);
    next_id++;
    contacts[new_contact.id()] = new_contact;
    response->set_success(true);
    return ::grpc::Status::OK;
}

::grpc::Status PhonebookServiceImpl::RemoveContact(::grpc::ServerContext* context, const ::ContactId* request, ::Status* response) {
    int id = request->id();
    if (contacts.count(id)) {
        contacts.erase(id);
        response->set_success(true);
    } else {
        response->set_success(false);
        response->set_message("Contact not found");
    }
    return ::grpc::Status::OK;
}

::grpc::Status PhonebookServiceImpl::SearchContacts(::grpc::ServerContext* context, const ::SearchQuery* request, ::Contacts* response) {
    std::vector<Contact> results;
    std::string query = request->query();
    bool search_in_notes = request->search_in_notes();

    for (const auto& pair : contacts) {
        const Contact& contact = pair.second;
        if (contact.first_name().find(query) != std::string::npos ||
            contact.last_name().find(query) != std::string::npos ||
            contact.middle_name().find(query) != std::string::npos ||
            contact.phone_number().find(query) != std::string::npos ||
            (search_in_notes && contact.note().find(query) != std::string::npos)) {
            results.push_back(contact);
        }
    }

    for (const Contact& contact : results) {
        *response->add_contacts() = contact;
    }

    return ::grpc::Status::OK;
}

::grpc::Status PhonebookServiceImpl::GetContact(::grpc::ServerContext* context, const ::ContactId* request, ::Contact* response) {
    int id = request->id();
    if (contacts.count(id)) {
        *response = contacts[id];
        return ::grpc::Status::OK;
    } else {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND, "Contact not found");
    }
}

int main() {
    loadContactsFromFile();

    std::string server_address("0.0.0.0:50051");
    PhonebookServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();

    return 0;
}