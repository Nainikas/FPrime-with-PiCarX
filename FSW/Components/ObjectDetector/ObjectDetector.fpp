module Components {
    @ Component that handles commands for object detection.
    active component ObjectDetector {

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port
        @ Input port to receive log text from UdpReceiver
        async input port udpRecv: Drv.ByteStreamRecv

        ##############################################################################
        #### Uncomment the following examples to start customizing your component ####
        ##############################################################################

        @ Command to start processing UDP messages
        async command StartDetection(trigger: U32)

        @ Command to start receiving detection packets on F
        async command StopDetection(trigger: U32)

        @ Telemetry channel to track the number of detections
        telemetry DetectionCount: U32
        

        @ Object detected event (raised when a log is received)
        event ObjectDetected(
          objLog: string
        ) severity activity high format "Object Detected Start or Stopped: {}"

        @ Example port: receiving calls from the rate group
        sync input port run: Svc.Sched

        @ trigger parameter
        param trigger: U32


        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}