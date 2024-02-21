#include <third-party/httplib.h>
#include <third-party/json.h>

#include <iostream>
#include <vector>

using json = nlohmann::json;

int main() {

    httplib::Server serve;

    std::map<int, json> products;
    int new_id = 0;

    serve.Post("/product", [&](const httplib::Request &req, httplib::Response &res) {
            std::cout << "POST request:\n";
            std::cout << req.body << '\n';

            json product = (json::parse(req.body));
            product["id"] = ++new_id;
            products.emplace(new_id, product);
            res.set_content(product.dump(), "application/json");
            return res.status = httplib::StatusCode::OK_200;
    });

    serve.Get("/product/:id", [&](const httplib::Request &req, httplib::Response &res) {
            auto id = stoi(req.path_params.at("id"));

            std::cout << "GET " << id << " product request:\n";
            std::cout << req.body << '\n';
            if (products.contains(id)) {
                res.set_content(products[id].dump(), "application/json");
                return res.status = httplib::StatusCode::OK_200;
            } else {
                res.set_content("Product not found", "text/plain");
                return res.status = httplib::StatusCode::NotFound_404;
            }

    });

    serve.Put("/product/:id", [&](const httplib::Request &req, httplib::Response &res) {
            auto id = stoi(req.path_params.at("id"));

            std::cout << "PUT " << id << " product request:\n";
            std::cout << req.body << '\n';
            if (products.contains(id)) {
                json product = json::parse(req.body);

                for (auto& [key, value] : product.items()) {
                    if (key == "id") {
                        res.set_content("Product id is immutable", "text/plain");
                        return res.status = httplib::StatusCode::Forbidden_403;
                    }
                    products[id][key] = value;
                }

                res.set_content(products[id].dump(), "application/json");
                return res.status = httplib::StatusCode::OK_200;
            } else {
                res.set_content("Product not found", "text/plain");
                return res.status = httplib::StatusCode::NotFound_404;
            }
    });

    serve.Delete("/product/:id", [&](const httplib::Request &req, httplib::Response &res) {
            auto id = stoi(req.path_params.at("id"));

            std::cout << "DELETE " << id << " product request:\n";
            std::cout << req.body << '\n';
            auto it = products.find(id);
            if (it != products.end()) {
                res.set_content(it->second.dump(), "application/json");
                products.erase(it);
                return res.status = httplib::StatusCode::OK_200;
            } else {
                res.set_content("Product not found", "text/plain");
                return res.status = httplib::StatusCode::NotFound_404;
            }
    });

    serve.Get("/products", [&products](const httplib::Request &req, httplib::Response &res) {
            std::cout << "GET all products request:\n";
            std::cout << req.body << '\n';

            std::vector<json> v;

            for (auto [_, i] : products) {
                v.emplace_back(i);
            }

            res.set_content(json(v).dump(), "application/json");
            return res.status = httplib::StatusCode::OK_200;
    });

    serve.listen("127.0.0.1", 8080);
}
