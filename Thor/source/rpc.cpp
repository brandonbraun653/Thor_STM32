#include <Thor/include/rpc.hpp>


namespace Thor
{
	namespace eRPC
	{
		void erpcServerSetup(int serialChannel, void*(*service)(void))
		{
			/* Init the eRPC server environment */
			erpc_transport_t transport = erpc_transport_thor_serial_init(serialChannel);

			/* Message buffer factory initialization */
			erpc_mbf_t message_buffer_factory = erpc_mbf_dynamic_init();

			/* eRPC server side initialization */
			erpc_server_init(transport, message_buffer_factory);

			/* Connect generated service into server */
			erpc_add_service_to_server(service());
		}
	}
}