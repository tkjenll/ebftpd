#include <sstream>
#include "util/string.hpp"
#include "cmd/site/gadduser.hpp"
#include "cmd/site/adduser.hpp"

namespace cmd { namespace site
{

void GADDUSERCommand::Execute()
{
  std::string cpArgStr("GADDUSER ");
  cpArgStr += args[2];
  for (auto it = args.begin() + 3; it != args.end(); ++it)
    cpArgStr += " " + *it;

  std::vector<std::string> cpArgs;
  util::Split(cpArgs, cpArgStr, " ");
  ADDUSERCommand(client, cpArgStr, cpArgs).Execute(args[1]);
}

// end
}
} 

