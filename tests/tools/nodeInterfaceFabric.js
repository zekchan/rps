import child_process from 'child_process'
import Eos from 'eosjs-api'

const exec = script => new Promise((resolve, reject) => {
  child_process.exec(script, err => {
    if (err) {
      reject(err)
    } else {
      resolve()
    }
  })
})

export default ENDPOINT => {
  async function cleos(args, attemp = 0) {
    try {
      await exec(`docker exec -i eosio /opt/eosio/bin/cleos -u ${ENDPOINT} ${args}`)
    } catch (e) {
      if (e.message.indexOf('Transaction took too long') === -1) {
        throw e
      } else {
        await cleos(args, attemp + 1)
      }
    }
  }
  const eos = Eos({
    httpEndpoint: ENDPOINT,
  })
  return {
    eos,
    cleos
  }
}
