#include <iostream>
#include <boost/bind.hpp>
#include "pluginloader.h"
#include "server.h"
#include "utils.h"
#include "plugindb.h"

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <plugins path>" << std::endl;
    return 1;
  }

  PluginDb db;

  db.scan(argv[1]);

  PluginLoader loader(db, argv[1]);

  try {
    std::string path(Utils::getAddress());
    Server server(path, loader);

    boost::asio::signal_set signals(server.service(), SIGQUIT, SIGINT, SIGTERM);
    signals.async_wait([&server](const boost::system::error_code& error,
				 int signal_number) {
			 if (!error) {
			   std::cerr << "signal " << signal_number << ". quitting..." << std::endl;
			   server.shutdown();
			 }
		       });

    server.start();
    return server.loop();
  } catch (std::exception& ex) {
    std::cerr << "Failed to start server: " << ex.what() << std::endl;
    return 1;
  }
}
