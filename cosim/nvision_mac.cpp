#include "base_nvision_accelerator.hpp"

class nvision_accelerator : public base_nvision_accelerator {
private:
  /// Add your private fields here.

public:
  void __mac_4b(int a, int b, int c, int d) {
      signed char *a_cast = (signed char *)(&a);
      signed char *b_cast = (signed char *)(&b);
      signed char *c_cast = (signed char *)(&c);
      signed char *d_cast = (signed char *)(&d);

      this->__nvision_sim_accum += ((a_cast[0] << 4) >> 4) * ((b_cast[0] << 4) >> 4);
      this->__nvision_sim_accum += (a_cast[0] >> 4) * (b_cast[0] >> 4);

      this->__nvision_sim_accum += ((a_cast[1] << 4) >> 4) * ((b_cast[1] << 4) >> 4);
      this->__nvision_sim_accum += (a_cast[1] >> 4) * (b_cast[1] >> 4);

      this->__nvision_sim_accum += ((a_cast[2] << 4) >> 4) * ((b_cast[2] << 4) >> 4);
      this->__nvision_sim_accum += (a_cast[2] >> 4) * (b_cast[2] >> 4);

      this->__nvision_sim_accum += ((a_cast[3] << 4) >> 4) * ((b_cast[3] << 4) >> 4);
      this->__nvision_sim_accum += (a_cast[3] >> 4) * (b_cast[3] >> 4);


      this->__nvision_sim_accum += ((c_cast[0] << 4) >> 4) * ((d_cast[0] << 4) >> 4);
      this->__nvision_sim_accum += (c_cast[0] >> 4) * (d_cast[0] >> 4);

      this->__nvision_sim_accum += ((c_cast[1] << 4) >> 4) * ((d_cast[1] << 4) >> 4);
      this->__nvision_sim_accum += (c_cast[1] >> 4) * (d_cast[1] >> 4);

      this->__nvision_sim_accum += ((c_cast[2] << 4) >> 4) * ((d_cast[2] << 4) >> 4);
      this->__nvision_sim_accum += (c_cast[2] >> 4) * (d_cast[2] >> 4);

      this->__nvision_sim_accum += ((c_cast[3] << 4) >> 4) * ((d_cast[3] << 4) >> 4);
      this->__nvision_sim_accum += (c_cast[3] >> 4) * (d_cast[3] >> 4);
  }

  void __mac_8b(int a, int b, int c, int d) {
    signed char *a_cast = (signed char *)(&a);
    signed char *b_cast = (signed char *)(&b);
    signed char *c_cast = (signed char *)(&c);
    signed char *d_cast = (signed char *)(&d);

    this->__nvision_sim_accum += a_cast[0] * b_cast[0];
    this->__nvision_sim_accum += a_cast[1] * b_cast[1];
    this->__nvision_sim_accum += a_cast[2] * b_cast[2];
    this->__nvision_sim_accum += a_cast[3] * b_cast[3];

    this->__nvision_sim_accum += c_cast[0] * d_cast[0];
    this->__nvision_sim_accum += c_cast[1] * d_cast[1];
    this->__nvision_sim_accum += c_cast[2] * d_cast[2];
    this->__nvision_sim_accum += c_cast[3] * d_cast[3];
  }

  void __mac_16b(int a, int b, int c, int d) {
      short *a_cast = (short *)(&a);
      short *b_cast = (short *)(&b);
      short *c_cast = (short *)(&c);
      short *d_cast = (short *)(&d);

      this->__nvision_sim_accum += a_cast[0] * b_cast[0];
      this->__nvision_sim_accum += a_cast[1] * b_cast[1];

      this->__nvision_sim_accum += c_cast[0] * d_cast[0];
      this->__nvision_sim_accum += c_cast[1] * d_cast[1];
  }

  void __mac_32b(int a, int b, int c, int d) {
      this->__nvision_sim_accum += a * b;
      this->__nvision_sim_accum += c * d;
  }

  int __read_clear() {
    int temp = this->__nvision_sim_accum;
    this->__nvision_sim_accum = 0;

    return temp;
  }

  void busy (const socsim::simulation_data &in, socsim::simulation_data &out) override {
    uint32_t instr = this->get_instr();
    uint8_t funct3 = static_cast<uint8_t>((instr >> 12) & 0x7);

    switch (funct3) {
      case 0x0: { // MAC_4B
        std::cout << "Executing MAC_4B (rs1 = " << this->get_rs1_val() << ", rs2 = " << this->get_rs2_val() << ")" << std::endl;
        this->parts_received++;
        if (this->parts_received >= 2) {
          this->parts_received = 0;
          __mac_4b(this->get_internal_rs1(), this->get_internal_rs2(), this->get_rs1_val(), this->get_rs2_val());
          this->set_result_val(0); // Unused
        } else {
          this->set_internal_rs1(this->get_rs1_val());
          this->set_internal_rs2(this->get_rs2_val());
          this->set_result_val(0); // Unused
        }
        break;
      }
      case 0x2: { // MAC_8B
        std::cout << "Executing MAC_8B (rs1 = " << this->get_rs1_val() << ", rs2 = " << this->get_rs2_val() << ")" << std::endl;
        this->parts_received++;
        if (this->parts_received >= 2) {
          this->parts_received = 0;
          __mac_8b(this->get_internal_rs1(), this->get_internal_rs2(), this->get_rs1_val(), this->get_rs2_val());
          this->set_result_val(0); // Unused
        } else {
          this->set_internal_rs1(this->get_rs1_val());
          this->set_internal_rs2(this->get_rs2_val());
          this->set_result_val(0); // Unused
        }
        break;
      }
      case 0x4: { // MAC_16B
        std::cout << "Executing MAC_16B (rs1 = " << this->get_rs1_val() << ", rs2 = " << this->get_rs2_val() << ")" << std::endl;
        this->parts_received++;
        if (this->parts_received >= 2) {
          this->parts_received = 0;
          __mac_16b(this->get_internal_rs1(), this->get_internal_rs2(), this->get_rs1_val(), this->get_rs2_val());
          this->set_result_val(0); // Unused
        } else {
          this->set_internal_rs1(this->get_rs1_val());
          this->set_internal_rs2(this->get_rs2_val());
          this->set_result_val(0); // Unused
        }
        break;
      }
      case 0x6: { // MAC_32B
        std::cout << "Executing MAC_32B (rs1 = " << this->get_rs1_val() << ", rs2 = " << this->get_rs2_val() << ")" << std::endl;
        this->parts_received++;
        if (this->parts_received >= 2) {
          this->parts_received = 0;
          __mac_32b(this->get_internal_rs1(), this->get_internal_rs2(), this->get_rs1_val(), this->get_rs2_val());
          this->set_result_val(0); // Unused
        } else {
          this->set_internal_rs1(this->get_rs1_val());
          this->set_internal_rs2(this->get_rs2_val());
          this->set_result_val(0); // Unused
        }
        break;
      }
      case 0x7: { // READ_CLEAR
        uint32_t result = __read_clear();
        std::cout << "Reading and clearing accumulator: " << result << std::endl;
        this->set_result_val(result);
        break;
      }
      default: {
        this->set_result_val(0);
        break;
      }
    }
  }
};

int main () {
  nvision_accelerator simulator;

  nvision_accelerator_comms comms ("tcp://*:5556");
  comms.connect();

  while (true){
    socsim::simulation_data inputs = comms.recv();

    socsim::simulation_data outputs = simulator.step(inputs);

    comms.send(outputs);
  }

  comms.shutdown();

  return EXIT_SUCCESS;
}
