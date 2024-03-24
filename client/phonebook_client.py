from tkinter import messagebox
import grpc
import phonebook_pb2
import phonebook_pb2_grpc
from tkinter import *

class PhonebookClient:
    def __init__(self, host='localhost', port=50051):
        self.channel = grpc.insecure_channel(f'{host}:{port}')
        self.stub = phonebook_pb2_grpc.PhonebookServiceStub(self.channel)

    def add_contact(self, contact):
        response = self.stub.AddContact(contact)
        return response.success, response.message

    def remove_contact(self, contact_id):
        response = self.stub.RemoveContact(phonebook_pb2.ContactId(id=contact_id))
        return response.success, response.message

    def search_contacts(self, query, search_in_notes=False):
        response = self.stub.SearchContacts(phonebook_pb2.SearchQuery(query=query, search_in_notes=search_in_notes))
        return response.contacts

    def get_contact(self, contact_id):
        response = self.stub.GetContact(phonebook_pb2.ContactId(id=contact_id))
        return response

class PhonebookGUI(Tk):
    def __init__(self, client):
        super().__init__()
        self.client = client
        self.title("Phonebook Client")

        # Создание виджетов
        self.name_label = Label(self, text="Name:")
        self.name_entry = Entry(self)
        self.phone_label = Label(self, text="Phone:")
        self.phone_entry = Entry(self)
        self.note_label = Label(self, text="Note:")
        self.note_text = Text(self, height=5)
        self.add_button = Button(self, text="Add", command=self.add_contact)
        self.remove_button = Button(self, text="Remove", command=self.remove_contact)
        self.search_label = Label(self, text="Search:")
        self.search_entry = Entry(self)
        self.search_button = Button(self, text="Search", command=self.search_contacts)
        self.results_list = Listbox(self, height=10)
        self.results_list.bind('<<ListboxSelect>>', self.show_contact)

        # Размещение виджетов
        self.name_label.grid(row=0, column=0)
        self.name_entry.grid(row=0, column=1)
        self.phone_label.grid(row=1, column=0)
        self.phone_entry.grid(row=1, column=1)
        self.note_label.grid(row=2, column=0)
        self.note_text.grid(row=2, column=1)
        self.add_button.grid(row=3, column=0)
        self.remove_button.grid(row=3, column=1)
        self.search_label.grid(row=4, column=0)
        self.search_entry.grid(row=4, column=1)
        self.search_button.grid(row=5, column=0)
        self.results_list.grid(row=5, column=1)

    def add_contact(self):
        name = self.name_entry.get()
        phone = self.phone_entry.get()
        note = self.note_text.get("1.0", "end-1c")
        contact = phonebook_pb2.Contact(first_name=name, phone_number=phone, note=note)
        success, message = self.client.add_contact(contact)
        if success:
            messagebox.showinfo("Success", "Contact added successfully")
            self.name_entry.delete(0, END)
            self.phone_entry.delete(0, END)
            self.note_text.delete("1.0", END)
        else:
            messagebox.showerror("Error", message)

    def remove_contact(self):
        selected = self.results_list.curselection()
        if selected:
            contact_id = self.results_list.get(selected[0]).split(" - ")[1]
            success, message = self.client.remove_contact(int(contact_id))
            if success:
                messagebox.showinfo("Success", "Contact removed successfully")
                self.results_list.delete(selected[0])
            else:
                messagebox.showerror("Error", message)
        else:
            messagebox.showerror("Error", "Please select a contact to remove")

    def search_contacts(self):
        query = self.search_entry.get()
        contacts = self.client.search_contacts(query)
        self.results_list.delete(0, END)
        for contact in contacts:
            self.results_list.insert(END, f"{contact.first_name} {contact.last_name} - {contact.phone_number}")
    
    def show_contact(self, event):
        selected = self.results_list.curselection()
        if selected:
            contact_id = self.results_list.get(selected[0]).split(" - ")[1]
            contact = self.client.get_contact(int(contact_id))
            messagebox.showinfo("Contact Details", f"Name: {contact.first_name} {contact.last_name} {contact.middle_name}\nPhone: {contact.phone_number}\nNote: {contact.note}")


if __name__ == '__main__':
    client = PhonebookClient()
    app = PhonebookGUI(client)
    app.mainloop()