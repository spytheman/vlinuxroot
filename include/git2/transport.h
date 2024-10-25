/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_git_transport_h__
#define INCLUDE_git_transport_h__

#include "indexer.h"
#include "net.h"
#include "types.h"

/**
 * @file git2/transport.h
 * @brief Git transport interfaces and functions
 * @defgroup git_transport interfaces and functions
 * @ingroup Git
 * @{
 */
GIT_BEGIN_DECL

/** Signature of a function which creates a transport */
typedef int GIT_CALLBACK(git_transport_cb)(git_transport **out, git_remote *owner, void *param);

/**
 * Type of SSH host fingerprint
 */
typedef enum {
	/** MD5 is available */
	GIT_CERT_SSH_MD5 = (1 << 0),
	/** SHA-1 is available */
	GIT_CERT_SSH_SHA1 = (1 << 1),
	/** SHA-256 is available */
	GIT_CERT_SSH_SHA256 = (1 << 2),
	/** Raw hostkey is available */
	GIT_CERT_SSH_RAW = (1 << 3)
} git_cert_ssh_t;

typedef enum {
	/** The raw key is of an unknown type. */
	GIT_CERT_SSH_RAW_TYPE_UNKNOWN = 0,
	/** The raw key is an RSA key. */
	GIT_CERT_SSH_RAW_TYPE_RSA = 1,
	/** The raw key is a DSS key. */
	GIT_CERT_SSH_RAW_TYPE_DSS = 2,
	/** The raw key is a ECDSA 256 key. */
	GIT_CERT_SSH_RAW_TYPE_KEY_ECDSA_256 = 3,
	/** The raw key is a ECDSA 384 key. */
	GIT_CERT_SSH_RAW_TYPE_KEY_ECDSA_384 = 4,
	/** The raw key is a ECDSA 521 key. */
	GIT_CERT_SSH_RAW_TYPE_KEY_ECDSA_521 = 5,
	/** The raw key is a ED25519 key. */
	GIT_CERT_SSH_RAW_TYPE_KEY_ED25519 = 6
} git_cert_ssh_raw_type_t;

/**
 * Hostkey information taken from libssh2
 */
typedef struct {
	git_cert parent;

	/**
	 * A hostkey type from libssh2, either
	 * `GIT_CERT_SSH_MD5` or `GIT_CERT_SSH_SHA1`
	 */
	git_cert_ssh_t type;

	/**
	 * Hostkey hash. If type has `GIT_CERT_SSH_MD5` set, this will
	 * have the MD5 hash of the hostkey.
	 */
	unsigned char hash_md5[16];

	/**
	 * Hostkey hash. If type has `GIT_CERT_SSH_SHA1` set, this will
	 * have the SHA-1 hash of the hostkey.
	 */
	unsigned char hash_sha1[20];

	/**
	 * Hostkey hash. If `type` has `GIT_CERT_SSH_SHA256` set, this will
	 * have the SHA-256 hash of the hostkey.
	 */
	unsigned char hash_sha256[32];

	/**
	 * Raw hostkey type. If `type` has `GIT_CERT_SSH_RAW` set, this will
	 * have the type of the raw hostkey.
	 */
	git_cert_ssh_raw_type_t raw_type;

	/**
	 * Pointer to the raw hostkey. If `type` has `GIT_CERT_SSH_RAW` set,
	 * this will have the raw contents of the hostkey.
	 */
	const char *hostkey;

	/**
	 * Raw hostkey length. If `type` has `GIT_CERT_SSH_RAW` set, this will
	 * have the length of the raw contents of the hostkey.
	 */
	size_t hostkey_len;
} git_cert_hostkey;

/**
 * X.509 certificate information
 */
typedef struct {
	git_cert parent;
	/**
	 * Pointer to the X.509 certificate data
	 */
	void *data;
	/**
	 * Length of the memory block pointed to by `data`.
	 */
	size_t len;
} git_cert_x509;

/*
 *** Begin interface for credentials acquisition ***
 */

/**
 * Supported credential types
 *
 * This represents the various types of authentication methods supported by
 * the library.
 */
typedef enum {
	/**
	 * A vanilla user/password request
	 * @see git_cred_userpass_plaintext_new
	 */
	GIT_CREDTYPE_USERPASS_PLAINTEXT = (1u << 0),

	/**
	 * An SSH key-based authentication request
	 * @see git_cred_ssh_key_new
	 */
	GIT_CREDTYPE_SSH_KEY = (1u << 1),

	/**
	 * An SSH key-based authentication request, with a custom signature
	 * @see git_cred_ssh_custom_new
	 */
	GIT_CREDTYPE_SSH_CUSTOM = (1u << 2),

	/**
	 * An NTLM/Negotiate-based authentication request.
	 * @see git_cred_default
	 */
	GIT_CREDTYPE_DEFAULT = (1u << 3),

	/**
	 * An SSH interactive authentication request
	 * @see git_cred_ssh_interactive_new
	 */
	GIT_CREDTYPE_SSH_INTERACTIVE = (1u << 4),

	/**
	 * Username-only authentication request
	 *
	 * Used as a pre-authentication step if the underlying transport
	 * (eg. SSH, with no username in its URL) does not know which username
	 * to use.
	 *
	 * @see git_cred_username_new
	 */
	GIT_CREDTYPE_USERNAME = (1u << 5),

	/**
	 * An SSH key-based authentication request
	 *
	 * Allows credentials to be read from memory instead of files.
	 * Note that because of differences in crypto backend support, it might
	 * not be functional.
	 *
	 * @see git_cred_ssh_key_memory_new
	 */
	GIT_CREDTYPE_SSH_MEMORY = (1u << 6),
} git_credtype_t;

typedef struct git_cred git_cred;

/**
 * The base structure for all credential types
 */
struct git_cred {
	git_credtype_t credtype; /**< A type of credential */
	void GIT_CALLBACK(free)(git_cred *cred);
};

/** A plaintext username and password */
typedef struct {
	git_cred parent;
	char *username;
	char *password;
} git_cred_userpass_plaintext;


/*
 * If the user hasn't included libssh2.h before git2.h, we need to
 * define a few types for the callback signatures.
 */
#ifndef LIBSSH2_VERSION
typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;
typedef struct _LIBSSH2_USERAUTH_KBDINT_PROMPT LIBSSH2_USERAUTH_KBDINT_PROMPT;
typedef struct _LIBSSH2_USERAUTH_KBDINT_RESPONSE LIBSSH2_USERAUTH_KBDINT_RESPONSE;
#endif

typedef int GIT_CALLBACK(git_cred_sign_callback)(LIBSSH2_SESSION *session, unsigned char **sig, size_t *sig_len, const unsigned char *data, size_t data_len, void **abstract);
typedef void GIT_CALLBACK(git_cred_ssh_interactive_callback)(const char* name, int name_len, const char* instruction, int instruction_len, int num_prompts, const LIBSSH2_USERAUTH_KBDINT_PROMPT* prompts, LIBSSH2_USERAUTH_KBDINT_RESPONSE* responses, void **abstract);

/**
 * A ssh key from disk
 */
typedef struct git_cred_ssh_key {
	git_cred parent;
	char *username;
	char *publickey;
	char *privatekey;
	char *passphrase;
} git_cred_ssh_key;

/**
 * Keyboard-interactive based ssh authentication
 */
typedef struct git_cred_ssh_interactive {
	git_cred parent;
	char *username;
	git_cred_ssh_interactive_callback prompt_callback;
	void *payload;
} git_cred_ssh_interactive;

/**
 * A key with a custom signature function
 */
typedef struct git_cred_ssh_custom {
	git_cred parent;
	char *username;
	char *publickey;
	size_t publickey_len;
	git_cred_sign_callback sign_callback;
	void *payload;
} git_cred_ssh_custom;

/** A key for NTLM/Kerberos "default" credentials */
typedef struct git_cred git_cred_default;

/** Username-only credential information */
typedef struct git_cred_username {
	git_cred parent;
	char username[1];
} git_cred_username;

/**
 * Check whether a credential object contains username information.
 *
 * @param cred object to check
 * @return 1 if the credential object has non-NULL username, 0 otherwise
 */
GIT_EXTERN(int) git_cred_has_username(git_cred *cred);

/**
 * Create a new plain-text username and password credential object.
 * The supplied credential parameter will be internally duplicated.
 *
 * @param out The newly created credential object.
 * @param username The username of the credential.
 * @param password The password of the credential.
 * @return 0 for success or an error code for failure
 */
GIT_EXTERN(int) git_cred_userpass_plaintext_new(
	git_cred **out,
	const char *username,
	const char *password);

/**
 * Create a new passphrase-protected ssh key credential object.
 * The supplied credential parameter will be internally duplicated.
 *
 * @param out The newly created credential object.
 * @param username username to use to authenticate
 * @param publickey The path to the public key of the credential.
 * @param privatekey The path to the private key of the credential.
 * @param passphrase The passphrase of the credential.
 * @return 0 for success or an error code for failure
 */
GIT_EXTERN(int) git_cred_ssh_key_new(
	git_cred **out,
	const char *username,
	const char *publickey,
	const char *privatekey,
	const char *passphrase);

/**
 * Create a new ssh keyboard-interactive based credential object.
 * The supplied credential parameter will be internally duplicated.
 *
 * @param username Username to use to authenticate.
 * @param prompt_callback The callback method used for prompts.
 * @param payload Additional data to pass to the callback.
 * @return 0 for success or an error code for failure.
 */
GIT_EXTERN(int) git_cred_ssh_interactive_new(
	git_cred **out,
	const char *username,
	git_cred_ssh_interactive_callback prompt_callback,
	void *payload);

/**
 * Create a new ssh key credential object used for querying an ssh-agent.
 * The supplied credential parameter will be internally duplicated.
 *
 * @param out The newly created credential object.
 * @param username username to use to authenticate
 * @return 0 for success or an error code for failure
 */
GIT_EXTERN(int) git_cred_ssh_key_from_agent(
	git_cred **out,
	const char *username);

/**
 * Create an ssh key credential with a custom signing function.
 *
 * This lets you use your own function to sign the challenge.
 *
 * This function and its credential type is provided for completeness
 * and wraps `libssh2_userauth_publickey()`, which is undocumented.
 *
 * The supplied credential parameter will be internally duplicated.
 *
 * @param out The newly created credential object.
 * @param username username to use to authenticate
 * @param publickey The bytes of the public key.
 * @param publickey_len The length of the public key in bytes.
 * @param sign_callback The callback method to sign the data during the challenge.
 * @param payload Additional data to pass to the callback.
 * @return 0 for success or an error code for failure
 */
GIT_EXTERN(int) git_cred_ssh_custom_new(
	git_cred **out,
	const char *username,
	const char *publickey,
	size_t publickey_len,
	git_cred_sign_callback sign_callback,
	void *payload);

/**
 * Create a "default" credential usable for Negotiate mechanisms like NTLM
 * or Kerberos authentication.
 *
 * @return 0 for success or an error code for failure
 */
GIT_EXTERN(int) git_cred_default_new(git_cred **out);

/**
 * Create a credential to specify a username.
 *
 * This is used with ssh authentication to query for the username if
 * none is specified in the url.
 */
GIT_EXTERN(int) git_cred_username_new(git_cred **cred, const char *username);

/**
 * Create a new ssh key credential object reading the keys from memory.
 *
 * @param out The newly created credential object.
 * @param username username to use to authenticate.
 * @param publickey The public key of the credential.
 * @param privatekey The private key of the credential.
 * @param passphrase The passphrase of the credential.
 * @return 0 for success or an error code for failure
 */
GIT_EXTERN(int) git_cred_ssh_key_memory_new(
	git_cred **out,
	const char *username,
	const char *publickey,
	const char *privatekey,
	const char *passphrase);


/**
 * Free a credential.
 *
 * This is only necessary if you own the object; that is, if you are a
 * transport.
 *
 * @param cred the object to free
 */
GIT_EXTERN(void) git_cred_free(git_cred *cred);

/**
 * Signature of a function which acquires a credential object.
 *
 * @param cred The newly created credential object.
 * @param url The resource for which we are demanding a credential.
 * @param username_from_url The username that was embedded in a "user\@host"
 *                          remote url, or NULL if not included.
 * @param allowed_types A bitmask stating which cred types are OK to return.
 * @param payload The payload provided when specifying this callback.
 * @return 0 for success, < 0 to indicate an error, > 0 to indicate
 *       no credential was acquired
 */
typedef int GIT_CALLBACK(git_cred_acquire_cb)(
	git_cred **cred,
	const char *url,
	const char *username_from_url,
	unsigned int allowed_types,
	void *payload);

/** @} */
GIT_END_DECL
#endif
