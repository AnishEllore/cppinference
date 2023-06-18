package main

import (
	"flag"
	"fmt"
	"io"
	"log"
	"math"
	"net/http"
	"net/url"
	"os/exec"
	"sort"
	"strconv"
	"time"
)

// global container name
var containerName string

func main() {
	cName := flag.String("c", "", "Container name")
	flag.Parse()
	containerName = *cName
	// define origin server URL
	originServerURL, err := url.Parse("http://127.0.0.1:7000")
	if err != nil {
		log.Fatal("invalid origin server URL")
	}

	// Channels for communication
	dataCh := make(chan time.Duration)
	controlCh := make(chan struct{})
	doneCh := make(chan struct{})

	// Initial vCPU allocation
	vCPUAllocation := 1

	// Start the reader (monitorSLA)
	go monitorSLA(dataCh, &vCPUAllocation, controlCh, doneCh)

	// Start the reverse proxy server
	reverseProxy := http.HandlerFunc(func(rw http.ResponseWriter, req *http.Request) {
		startTime := time.Now()

		// set req Host, URL and Request URI to forward a request to the origin server
		req.Host = originServerURL.Host
		req.URL.Host = originServerURL.Host
		req.URL.Scheme = originServerURL.Scheme
		req.RequestURI = ""

		// save the response from the origin server
		originServerResponse, err := http.DefaultClient.Do(req)
		if err != nil {
			rw.WriteHeader(http.StatusInternalServerError)
			_, _ = fmt.Fprint(rw, err)
			return
		}

		// Measure latency and send to the data channel
		latency := time.Since(startTime)
		dataCh <- latency

		// Write the latency as a response header
		//rw.Header().Set("Latency", latency.String())

		// Write the response to the client
		rw.WriteHeader(http.StatusOK)
		io.Copy(rw, originServerResponse.Body)
	})
	go func() {
		log.Fatal(http.ListenAndServe(":8080", reverseProxy))
	}()

	// Wait for the reader (monitorSLA) to finish
	<-doneCh

	fmt.Println("Reader has completed.")
}

// monitorSLA continuously monitors the SLA violation and adjusts the vCPU allocation if necessary
func monitorSLA(dataCh <-chan time.Duration, vCPUAllocation *int, controlCh <-chan struct{}, doneCh chan<- struct{}) {
	var latencies []time.Duration
	var checkSLA bool

	ticker := time.NewTicker(5 * time.Second) // Adjust the interval as needed
	defer ticker.Stop()

	for {
		select {
		case latency := <-dataCh:
			// Add latency to the slice
			latencies = append(latencies, latency)

			// Print latency and length of the slice
			fmt.Printf("Latency: %v, Latency samples: %d\n", latency, len(latencies))

		case <-ticker.C:
			// Check SLA violation occasionally
			checkSLA = true

		case <-controlCh:
			doneCh <- struct{}{} // Signal the completion of the reader (monitorSLA) to the main goroutine
			return
		}

		// Check SLA violation if necessary
		if checkSLA {
			checkSLA = false

			if len(latencies) == 0 {
				// print message and length of the slice
				fmt.Println("Insufficient latency data. Waiting...")
				fmt.Printf("Latency samples: %d\n", len(latencies))
				continue
			}

			// Calculate and print latency percentiles
			sort.Slice(latencies, func(i, j int) bool {
				return latencies[i] < latencies[j]
			})

			p90Index := int((90.0 / 100) * float64(len(latencies)-1))
			p90Latency := latencies[p90Index]
			if p90Latency > 300*time.Millisecond {
				fmt.Println("SLA violated: p90 latency exceeded 300 ms")
				if *vCPUAllocation >= 56 {
					continue
				}
				// Increase vCPU allocation by a multiple of 2
				*vCPUAllocation *= 2
				// set the maximum vCPU allocation to 56
				*vCPUAllocation = int(math.Min(float64(*vCPUAllocation), 56))
				fmt.Printf("Increased vCPU allocation to %d CPUs\n", *vCPUAllocation)

				// Update the vCPU allocation of the container
				err := updateContainerCPUs(*vCPUAllocation)
				if err != nil {
					log.Println("Failed to update container CPUs:", err)
				}
			}
		}
	}
}

// updateContainerCPUs updates the vCPU allocation of the container using Docker CLI
func updateContainerCPUs(cpus int) error {
	cmd := exec.Command("docker", "update", "--cpus", strconv.Itoa(cpus), containerName)
	err := cmd.Run()
	return err
}




