all: sample

sample_flash: sample
	@$(MAKE) -C sample flash

sample: library
	@$(MAKE) -C sample all

library:
	@$(MAKE) -C libled all

clean:
	@$(MAKE) -C sample clean
	@$(MAKE) -C libled clean