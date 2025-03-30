// ======================================================================
// \title  ObjectDetector.hpp
// \brief  hpp file for ObjectDetector component implementation class
// ======================================================================

#ifndef Components_ObjectDetector_HPP
#define Components_ObjectDetector_HPP

#include "Components/ObjectDetector/ObjectDetectorComponentAc.hpp"
#include <FpConfig.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Buffer/Buffer.hpp>
#include <atomic>
#include <thread>

namespace Components {

  class ObjectDetectorComponentImpl : public ObjectDetectorComponentBase {
    public:
      //! Construct ObjectDetector object
      ObjectDetectorComponentImpl(const char* const compName);
      //! Destroy ObjectDetector object
      ~ObjectDetectorComponentImpl();

      // Command handler for StartDetection (triggering the Pi to start detection)
      void StartDetection_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 trigger) override;

      // Command handler for StopDetection (stopping detection when trigger is 0)
      void StopDetection_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 trigger) override;

      // UDP receive handler (not used with our custom receiver, provided for interface compliance)
      void udpRecv_handler(FwIndexType portNum, Fw::Buffer &recvBuffer, const Drv::RecvStatus &recvStatus) override;

      // Optional periodic run_handler (if needed for telemetry)
      void run_handler(FwIndexType portNum, U32 context) override;

    PRIVATE:
      U32 m_detectionCount;
      // Members for the custom UDP receiver thread.
      std::atomic<bool> m_stopUdpReceiver;
      std::thread m_udpReceiverThread;

      // Custom UDP receiver thread function.
      void udpReceiverThreadFunc();
  };

  using ObjectDetector = ObjectDetectorComponentImpl;
}

#endif
