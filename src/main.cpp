#include <iostream>
#include <boost/bind.hpp>
#include "pluginloader.h"
#include "server.h"
#include "utils.h"
#include "plugindb.h"
#include <boost/program_options.hpp>

int main(int argc, char *argv[]) {
  boost::program_options::options_description desc("Allowed options");
  boost::program_options::variables_map vm;

  desc.add_options()
    ("help,h", "Show help message")
    ("plugins,p", boost::program_options::value<std::string>(), "plugins path")
    ("loaders,l", boost::program_options::value<std::string>(), "loaderss path")
    ;

  try {
    boost::program_options::store(boost::program_options::parse_command_line(argc,
									     argv, desc), vm);
  } catch (std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return 1;
  }

  boost::program_options::notify(vm);

  if (vm.count("help")) {
    std::cout << desc;
    return 0;
  }

  if (!vm.count("plugins")) {
    std::cerr << "plugins path is required" << std::endl << desc;
    return 1;
  }

  if (!vm.count("loaders")) {
    std::cerr << "loaders path is required" << std::endl << desc;
    return 1;
  }

  PluginDb db;

  db.scan(vm["plugins"].as<std::string>());

  PluginLoader loader(db, vm["plugins"].as<std::string>(), vm["loaders"].as<std::string>());

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
