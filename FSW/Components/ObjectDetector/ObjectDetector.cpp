// ======================================================================
// \title  ObjectDetector.cpp
// \brief  Custom UDP-based ObjectDetector component that starts/stops detection.
//         Uses StartDetection to trigger (trigger = 1) the Pi and
//         StopDetection (trigger = 0) to halt detection. The UDP server
//         binds to 0.0.0.0:6000.
// ======================================================================

#include "Components/ObjectDetector/ObjectDetector.hpp"
#include <Fw/Log/LogString.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Buffer/Buffer.hpp>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <sys/time.h>

namespace Components {

  ObjectDetectorComponentImpl::ObjectDetectorComponentImpl(const char* const compName)
      : ObjectDetectorComponentBase(compName)
      , m_detectionCount(0)
      , m_stopUdpReceiver(false)
  {
  }

  ObjectDetectorComponentImpl::~ObjectDetectorComponentImpl() {
      m_stopUdpServer = true;
      if (m_udpReceiverThread.joinable()) {
          m_udpReceiverThread.join();
      }
  }

  // ----------------------------------------------------------------------
  // Custom UDP server thread function.
  // Binds to 0.0.0.0:6000 (all interfaces) and continuously listens for detection messages.
  // Each received message is logged as an event.
  // ----------------------------------------------------------------------
  void ObjectDetectorComponentImpl::udpServerThreadFunc() {
      int sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock < 0) {
          Fw::LogStringArg err("Custom UDP Server: Failed to create socket");
          this->log_ACTIVITY_HI_ObjectDetected(err);
          return;
      }
      // Set socket options for reuse.
      int yes = 1;
      if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
          perror("setsockopt SO_REUSEADDR failed");
      }
#ifdef SO_REUSEPORT
      if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) < 0) {
          perror("setsockopt SO_REUSEPORT failed");
      }
#endif
      // Bind to 0.0.0.0:6000.
      struct sockaddr_in localAddr;
      memset(&localAddr, 0, sizeof(localAddr));
      localAddr.sin_family = AF_INET;
      localAddr.sin_port = htons(6000);
      localAddr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
      if (bind(sock, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
          Fw::LogStringArg err("Custom UDP Server: Failed to bind to 0.0.0.0:6000");
          this->log_ACTIVITY_HI_ObjectDetected(err);
          close(sock);
          return;
      }
      // Set a 1-second timeout.
      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
          perror("setsockopt SO_RCVTIMEO failed");
      }
      char buffer[1024];
      while (!m_stopUdpServer) {
          memset(buffer, 0, sizeof(buffer));
          struct sockaddr_in senderAddr;
          socklen_t senderLen = sizeof(senderAddr);
          ssize_t recvd = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                   (struct sockaddr*)&senderAddr, &senderLen);
          if (recvd > 0) {
              buffer[recvd] = '\0';
              std::string received(buffer);
              printf("Custom UDP Receiver got: %s\n", received.c_str());
              Fw::LogStringArg logArg(received.c_str());
              this->log_ACTIVITY_HI_ObjectDetected(logArg);
              m_detectionCount++;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      close(sock);
  }

  // ----------------------------------------------------------------------
  // StartDetection Command Handler:
  // Sends a UDP trigger "1" to the Pi at 192.168.1.99:6000.
  // This command tells the Pi to start detection.
  // ----------------------------------------------------------------------
  void ObjectDetectorComponentImpl::StartDetection_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 trigger) {
      Fw::LogStringArg logArg("StartDetection command: sending trigger '1' to Pi 192.168.1.99:6000");
      this->log_ACTIVITY_HI_ObjectDetected(logArg);
      int sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock < 0) {
          Fw::LogStringArg err("Failed to create UDP socket in StartDetection_cmdHandler");
          this->log_ACTIVITY_HI_ObjectDetected(err);
          this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
          return;
      }
      struct sockaddr_in piAddr;
      memset(&piAddr, 0, sizeof(piAddr));
      piAddr.sin_family = AF_INET;
      piAddr.sin_port = htons(6000);
      piAddr.sin_addr.s_addr = inet_addr("192.168.1.99");
      const char* triggerMsg = "1";
      ssize_t sent = sendto(sock, triggerMsg, strlen(triggerMsg), 0,
                              reinterpret_cast<struct sockaddr*>(&piAddr), sizeof(piAddr));
      if (sent < 0) {
          Fw::LogStringArg err("Failed to send UDP trigger '1' to PiCar");
          this->log_ACTIVITY_HI_ObjectDetected(err);
          this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
      } else {
          printf("Sent UDP trigger '1' to PiCar\n");
          this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
      }
      close(sock);
  }

  // ----------------------------------------------------------------------
  // StopDetection Command Handler:
  // Sends a UDP trigger "0" to the Pi at 192.168.1.99:6000 to stop detection,
  // then stops the local UDP server thread.
  // ----------------------------------------------------------------------
  void ObjectDetectorComponentImpl::StopDetection_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 trigger) {
      // We expect trigger==0 for stop command.
      if (trigger == 0) {
          Fw::LogStringArg logArg("StopDetection command: trigger 0 received, sending trigger '0' to Pi and stopping UDP reception");
          this->log_ACTIVITY_HI_ObjectDetected(logArg);
          
          int sock = socket(AF_INET, SOCK_DGRAM, 0);
          if (sock < 0) {
              Fw::LogStringArg err("Failed to create UDP socket in StopDetection_cmdHandler");
              this->log_ACTIVITY_HI_ObjectDetected(err);
          } else {
              struct sockaddr_in piAddr;
              memset(&piAddr, 0, sizeof(piAddr));
              piAddr.sin_family = AF_INET;
              piAddr.sin_port = htons(6000);
              piAddr.sin_addr.s_addr = inet_addr("192.168.1.99");
              const char* triggerMsg = "0";
              ssize_t sent = sendto(sock, triggerMsg, strlen(triggerMsg), 0,
                                      reinterpret_cast<struct sockaddr*>(&piAddr), sizeof(piAddr));
              if (sent < 0) {
                  Fw::LogStringArg err("Failed to send UDP trigger '0' to PiCar");
                  this->log_ACTIVITY_HI_ObjectDetected(err);
              } else {
                  printf("Sent UDP trigger '0' to PiCar\n");
              }
              close(sock);
          }
          
          m_stopUdpReceiver = true;
          if (m_udpServerThread.joinable()) {
              m_udpServerThread.join();
          }
      } else {
          Fw::LogStringArg logArg("StopDetection command: non-zero trigger received, ignoring");
          this->log_ACTIVITY_HI_ObjectDetected(logArg);
      }
      this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  // ----------------------------------------------------------------------
  // Unused built-in UDP handler (for interface compliance).
  // ----------------------------------------------------------------------
  void ObjectDetectorComponentImpl::udpRecv_handler(FwIndexType portNum, Fw::Buffer &recvBuffer, const Drv::RecvStatus &recvStatus) {
      const U8* data = recvBuffer.getData();
      std::string received(reinterpret_cast<const char*>(data));
      printf("udpRecv_handler triggered with data: %s\n", received.c_str());
      Fw::LogStringArg logArg(received.c_str());
      this->log_ACTIVITY_HI_ObjectDetected(logArg);
      m_detectionCount++;
  }

  // ----------------------------------------------------------------------
  // Optional run_handler for periodic telemetry (if needed).
  // ----------------------------------------------------------------------
  void ObjectDetectorComponentImpl::run_handler(FwIndexType portNum, U32 context) {
      // Not used in this simplified example.
  }

} // namespace Components
