#pragma once

#include <socsim/communication.hpp>
#include <socsim/memory.hpp>
#include <socsim/simulation_data.hpp>
#include <socsim/simulator.hpp>

class base_nvision_accelerator : public socsim::simulator {
protected:
    socsim::memory_controller controller;

    // Multi-cycle instruction support variables
    uint32_t internal_rs1 = 0;
    uint32_t internal_rs2 = 0;
    uint32_t parts_received = 0; // To track if all data has been received (Since they are sent in 2 steps)

    uint32_t rs1_val;
    uint32_t rs2_val;
    uint32_t instr;
    uint32_t result_val;

    signed int __nvision_sim_accum = 0;

    void extract_inputs(const socsim::simulation_data &inputs) {
        this->rs1_val = inputs.get_uint32 ("rs1_val");
        this->rs2_val = inputs.get_uint32 ("rs2_val");
        this->instr = inputs.get_uint32 ("instr");
    }

public:
    explicit base_nvision_accelerator()
        : rs1_val (0), rs2_val (0), instr (0) {
    }

    inline uint32_t get_internal_rs1 () const { return this->internal_rs1; }
    inline uint32_t get_internal_rs2 () const { return this->internal_rs2; }
    inline uint32_t get_parts_received () const { return this->parts_received; }

    inline uint32_t get_rs1_val () const { return this->rs1_val; } 
    inline uint32_t get_rs2_val () const { return this->rs2_val; } 
    inline uint32_t get_instr () const { return this->instr; } 

    inline void set_internal_rs1 (uint32_t rs1) { this->internal_rs1 = rs1; }
    inline void set_internal_rs2 (uint32_t rs2) { this->internal_rs2 = rs2; }

    inline void set_result_val (uint32_t result_val) { this->result_val = result_val; }

    virtual void busy(const socsim::simulation_data &in, socsim::simulation_data &out) = 0;

    socsim::simulation_data step(const socsim::simulation_data &data) override {
        socsim::simulation_data out = socsim::simulation_data({}, socsim::direction::TO_HOST);
        out.clear_memory_requests();
        this->extract_inputs(data);

        this->busy(data, out);

        out.clear_memory_requests();
        out.get_data()["result_val"] = this->result_val;

        return out;
    }
};

class nvision_accelerator_comms : public socsim::zmq_communication_handler {
public:
    explicit nvision_accelerator_comms(const std::string &address)
        : socsim::zmq_communication_handler(address) {
    }

    using zmq_communication_handler::connect;
    using zmq_communication_handler::recv;
    using zmq_communication_handler::send;
    using zmq_communication_handler::shutdown;

    std::unordered_map<std::string, std::any> recv_deserialize(const std::map<std::string, msgpack::object> &map) override {
        std::unordered_map<std::string, std::any> result;

        result["rs1_val"] = map.at ("rs1_val").as<std::uint32_t> ();
        result["rs2_val"] = map.at ("rs2_val").as<std::uint32_t> ();
        result["instr"] = map.at ("instr").as<std::uint32_t> ();

        return result;
    }

    std::map<std::string, msgpack::object> send_serialize(const socsim::simulation_data &data, msgpack::zone &zone) const override {
        std::map<std::string, msgpack::object> map;

        map["result_val"] = msgpack::object (data.get_uint32 ("result_val"), zone);

        return map;
    }
};
