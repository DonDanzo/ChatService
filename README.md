# ChatService
 Chat service helps to different user to communicate each other


Steps to build:

Download protobuf code from:
https://github.com/protocolbuffers/protobuf
and compile it.
After code is compiled, compile the message.proto file with command:
	(SolutionDir) is location where is located the solution.
	(ProtocDir) is location where is compiled protoc.exe
	1. run cmd prompt in directory of proto file	
	(ProtocDir)\protoc.exe --cpp_out=. messages.proto
		
	Building the solution with dependncies:
	Create Dependencies folder in (SolutionDir)
	with two sub folders Debug and Release
	
	
Create folder install one directory above (SolutionDir):
(SolutionDir)..\install
Download asio-1.30.2 from:
https://sourceforge.net/projects/asio/files/asio/1.30.2%20(Stable)/
and copy/move source files directory "src" to (SolutionDir)..\install
place in that folder compiled ProtoBuf compiled files with main folders:
absl
google

Finaly in (SolutionDir)..\install will have 3 main directories:
absl
asio
google


Copy next ProtoBuf compiled lib files (DEBUG) to Debug folder:		
																			 
libprotobufd.lib							absl_examine_stack.lib							absl_log_internal_globals.lib
libprotocd.lib                              absl_log_globals.lib                    		absl_strings_internal.lib
absl_log_internal_message.lib               absl_log_internal_format.lib            		absl_malloc_internal.lib
absl_log_internal_check_op.lib              absl_log_internal_log_sink_set.lib      		absl_graphcycles_internal.lib
absl_log_internal_nullguard.lib             absl_raw_hash_set.lib                   		absl_log_internal_proto.lib
absl_strings.lib                            absl_bad_variant_access.lib             		absl_int128.lib    
absl_str_format_internal.lib                absl_throw_delegate.lib                 		absl_crc_cord_state.lib    
absl_cord.lib                               absl_cordz_info.lib                     		absl_stacktrace.lib    
absl_city.lib                               absl_cord_internal.lib                  		absl_symbolize.lib    
absl_hash.lib                               absl_status.lib                         		absl_log_sink.lib    
utf8_validity.lib                           absl_statusor.lib                       		absl_low_level_hash.lib    
absl_raw_logging_internal.lib               absl_kernel_timeout_internal.lib        		absl_cordz_handle.lib    
absl_synchronization.lib                    absl_log_internal_conditions.lib        		absl_time_zone.lib    
absl_base.lib                               absl_time.lib                           		absl_crc32c.lib    
absl_spinlock_wait.lib                      absl_strerror.lib                       		absl_crc_internal.lib        
                                                                                   
     
Copy next ProtoBuf compiled lib files (Release) to Debug folder:

libprotobuf.lib								absl_examine_stack.lib							absl_log_internal_globals.lib
libprotoc.lib                             	absl_log_globals.lib                    		absl_strings_internal.lib
absl_log_internal_message.lib               absl_log_internal_format.lib            		absl_malloc_internal.lib
absl_log_internal_check_op.lib              absl_log_internal_log_sink_set.lib      		absl_graphcycles_internal.lib
absl_log_internal_nullguard.lib             absl_raw_hash_set.lib                   		absl_log_internal_proto.lib
absl_strings.lib                            absl_bad_variant_access.lib             		absl_int128.lib    
absl_str_format_internal.lib                absl_throw_delegate.lib                 		absl_crc_cord_state.lib    
absl_cord.lib                               absl_cordz_info.lib                     		absl_stacktrace.lib    
absl_city.lib                               absl_cord_internal.lib                  		absl_symbolize.lib    
absl_hash.lib                               absl_status.lib                         		absl_log_sink.lib    
utf8_validity.lib                           absl_statusor.lib                       		absl_low_level_hash.lib    
absl_raw_logging_internal.lib               absl_kernel_timeout_internal.lib        		absl_cordz_handle.lib    
absl_synchronization.lib                    absl_log_internal_conditions.lib        		absl_time_zone.lib    
absl_base.lib                               absl_time.lib                           		absl_crc32c.lib    
absl_spinlock_wait.lib                      absl_strerror.lib                       		absl_crc_internal.lib        






Usage:

ChatService solution builds 3 projects. Main libraly project ChatService, and two executable projects SpiderClient and SpiderServer.

After Client is started it asks to be inputed User Name with which will be connected to server, and asks should be used default IP and PORT to which will connect.
Default ip is "127.0.0.1" and the port is 11111. If user wan't to include hos own ip and port, must input Y. If no change is needed input N.
After that communication is started. If user has no activity 10 minutes, client will disconnect from the server.
Log file is created every hour, and all loged data is written in it.

After Server is started, it starts to listen for connections. If some client try to connect, it has to send special expected message.
If message is expected one, client is verified and connected. Each message send by client, is send to all other clients.
There is option for system messages like request some data, status or whatever, but it is still not implemented.

Hint:
There are a lot of log prints in std::cout turn on. If user wants to disable them, needs to change name DEBUG_MODE in Defs.h to whatever, example: _DEBUG_MODE









