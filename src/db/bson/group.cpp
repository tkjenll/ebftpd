#include <mongo/client/dbclient.h>
#include "db/bson/group.hpp"
#include "acl/group.hpp"
#include "db/bson/error.hpp"
#include "db/bson/bson.hpp"

namespace db { namespace bson
{

mongo::BSONObj Group::Serialize(const acl::Group& group)
{
  mongo::BSONObjBuilder bob;
  bob.append("modified", ToDateT(group.modified));
  bob.append("name", group.name);
  bob.append("gid", group.gid);
  return bob.obj();
}

void Group::Unserialize(const mongo::BSONObj& bo, acl::Group& group)
{
  try
  {
    group.modified = ToPosixTime(bo["modified"].Date());
    group.name = bo["name"].String();
    group.gid = bo["gid"].Int();
  }
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("group", e, bo);
  }
}

std::unique_ptr<acl::Group> Group::UnserializePtr(const mongo::BSONObj& bo)
{
  std::unique_ptr<acl::Group> group(new acl::Group());
  Unserialize(bo, *group);
  return group;
}

acl::Group Group::Unserialize(const mongo::BSONObj& bo)
{
  acl::Group group;
  Unserialize(bo, group);
  return group;
}

} /* bson namespace */
} /* db namespace */
