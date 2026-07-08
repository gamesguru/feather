#pragma once

#if __has_include(<openssl/base.h>)
#include <openssl/base.h>
#else
#include <openssl/ec.h>
#endif

/*
 * AWS-LC/BoringSSL does not expose EC_GROUP_check().
 * The EC certificate path uses a named built-in curve, so this validation is
 * redundant for the build variants we support here.
 */
#if defined(OPENSSL_IS_AWSLC) || defined(BORINGSSL_API_VERSION)
#undef EC_GROUP_check
#define EC_GROUP_check(group, ctx) 1
#else
#undef EC_GROUP_check
#define EC_GROUP_check(group, ctx) EC_GROUP_check_discriminant(group, ctx)
#endif
