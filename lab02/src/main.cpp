#include <third-party/httplib.h>
#include <third-party/json.h>

#include <iostream>
#include <vector>
#include <string>

using json = nlohmann::json;

int main() {

    httplib::Server serve;

    std::map<int, json> products;
    std::map<std::string, std::string> images;
    int new_id = 0;

    serve.Post("/product", [&](const httplib::Request &req, httplib::Response &res) {
            std::cerr << "POST request:\n";

            json product = (json::parse(req.body));
            product["id"] = ++new_id;
            products.emplace(new_id, product);
            res.set_content(product.dump(), "application/json");
            return res.status = httplib::StatusCode::OK_200;
    });

    serve.Get("/product/:id", [&](const httplib::Request &req, httplib::Response &res) {
            auto id = stoi(req.path_params.at("id"));

            std::cerr << "GET " << id << " product request:\n";
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

            std::cerr << "PUT " << id << " product request:\n";
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

            std::cerr << "DELETE " << id << " product request:\n";
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

    serve.Get("/products", [&products](const httplib::Request &, httplib::Response &res) {
            std::cerr << "GET all products request:\n";

            std::vector<json> v;

            for (auto [_, i] : products) {
                v.emplace_back(i);
            }

            res.set_content(json(v).dump(), "application/json");
            return res.status = httplib::StatusCode::OK_200;
    });

    serve.Post("/product/:id/image", [&](const httplib::Request &req, httplib::Response &res) {
            auto id = stoi(req.path_params.at("id"));

            std::cerr << "PUT image to product " << id << " request:\n";


            if (!products.contains(id)) {
                return res.status = httplib::StatusCode::NotFound_404;
            }
            if (!req.has_file("icon")) {
                res.set_content("`icon` form field not defined", "text/plain");
                return res.status = httplib::StatusCode::BadRequest_400;
            }
            auto file = req.get_file_value("icon");
            if (file.content_type != "image/png") {
                res.set_content("only png pictures is supported", "text/plain");
                return res.status = httplib::StatusCode::BadRequest_400;
            }

            products[id]["icon"] = file.filename;
            images[file.filename] = file.content;
            return res.status = httplib::StatusCode::OK_200;
    });

    serve.Get("/product/:id/image", [&](const httplib::Request &req, httplib::Response &res) {
            auto id = stoi(req.path_params.at("id"));

            std::cerr << "GET " << id << " product image request:\n";
            if (!products.contains(id) || !products[id].contains("icon")) {
                res.set_content("Product not found", "text/plain");
                return res.status = httplib::StatusCode::NotFound_404;
            }

            res.set_content(images[products[id]["icon"]], "image/png");
            return res.status = httplib::StatusCode::OK_200;
    });


    serve.listen("127.0.0.1", 8080);
}
