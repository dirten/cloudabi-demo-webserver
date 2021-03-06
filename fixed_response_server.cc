// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/socket.h>

#include <stdio.h>
#include <unistd.h>

#include <thread>

#include <arpc++/arpc++.h>
#include <flower/protocol/server.ad.h>

#include "fixed_response_server.h"
#include "logger.h"

using arpc::ServerContext;
using arpc::Status;
using flower::protocol::server::ConnectRequest;
using flower::protocol::server::ConnectResponse;

Status FixedResponseServer::Connect(ServerContext* context,
                                    const ConnectRequest* request,
                                    ConnectResponse* response) {
  logger_->Log() << "Received a connection!";

  // Process the request asynchronously.
  std::thread([ this, connection{request->client()} ]() {
    // Write a fixed HTTP response.
    dprintf(connection->get(),
            "HTTP/1.1 200 OK\r\n"
            "Connection: close\r\n"
            "Content-Length: %zu\r\n"
            "Content-Type: text/html; charset=utf-8\r\n\r\n",
            html_response_.size());
    write(connection->get(), html_response_.data(), html_response_.size());

    // Wait for the client to close the connection.
    shutdown(connection->get(), SHUT_WR);
    char discard[4096];
    while (read(connection->get(), discard, sizeof(discard)) > 0) {
    }
  })
      .detach();
  return Status::OK;
}
