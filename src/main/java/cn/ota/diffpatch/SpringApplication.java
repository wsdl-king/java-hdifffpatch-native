package cn.ota.diffpatch;

import hdiff.Hdiff;
import hpatch.Hpatch;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.cloud.client.discovery.EnableDiscoveryClient;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * @author qiwenshuai
 * @note
 * @since 18-12-7 14:08 by jdk 1.8
 */
@SpringBootApplication
@ComponentScan({"cn.ota.diffpatch.*", "hpatch", "hdiff"})
@EnableDiscoveryClient(autoRegister = false)
@RestController
public class SpringApplication {

    public static void main(String[] args) {
        org.springframework.boot.SpringApplication.run(SpringApplication.class, args);
    }

    @Bean
    public Hpatch getPatch() {
        return new Hpatch();
    }

    @Bean
    public Hdiff getDiff() {
        return new Hdiff();
    }

    @Autowired
    Hpatch hpatch;
    @Autowired
    Hdiff hdiff;

    @GetMapping(value = "/hello")
    public Integer dsds() {
        int hpatch = this.hpatch.hpatch("/home/qiwenshuai/chafen/A0/v0.9.tar", "", "/home/qiwenshuai/new.tar");
        return hpatch;
    }


}
