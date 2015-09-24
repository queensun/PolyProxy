#include "ProxySvc.h"

std::unique_ptr<ProxySvc::Acl> ProxySvc::defaultAcl = std::unique_ptr<ProxySvc::Acl>(new ProxySvc::Acl);