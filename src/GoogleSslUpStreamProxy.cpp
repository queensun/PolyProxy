#include "GoogleSslUpStreamProxy.h"

bool GoogleSslUpStreamProxy::check(const PeerID& peerID)
{
	std::string hostname;
	hostname.resize(peerID.ssl.hostname.size());

	std::transform(peerID.ssl.hostname.begin(), peerID.ssl.hostname.end(), 
		hostname.begin(), std::tolower);

	return
		hostname.find("google") != hostname.npos ||
		hostname.find("youtube.com") != hostname.npos ||
		hostname.find(".appspot.") != hostname.npos
		;
}